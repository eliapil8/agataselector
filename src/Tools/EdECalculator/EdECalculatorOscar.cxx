#include "EdECalculatorOscar.h"

#include "OscarConf.h"
#include "ReactionConf.h"
#include "ReplayConf.h"
#include "TargetConf.h"

#include <TCanvas.h>
#include <TColor.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TH2F.h>
#include <TLegend.h>
#include <TObject.h>
#include <TVector3.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace
{
using ConfigurationError = EdECalculator::EdEConfigurationError;

bool isValidHexColor(const std::string &input)
{
    if (input.size() != 7 || input.front() != '#')
        return false;
    return std::all_of(input.begin() + 1, input.end(),
                       [](unsigned char character) { return std::isxdigit(character); });
}

Color_t GetColor(int getColorIndex = -1, std::string hexColor = "")
{
    static std::mutex           colorMutex;
    std::lock_guard<std::mutex> lock(colorMutex);
    static unsigned int         colorIndex{0};
    static std::vector<std::string> colorArray{
        "#2196f3", "#f44336", "#3f51b5", "#4caf50", "#ff9800", "#000000", "#e91e63",
        "#8bc34a", "#ff5722", "#795548", "#607d8b", "#9c27b0", "#00bcd4", "#ffeb3b",
        "#673ab7", "#03a9f4", "#009688", "#cddc39", "#ffc107", "#9e9e9e"};
    if (!hexColor.empty() && isValidHexColor(hexColor))
        colorArray.emplace_back(std::move(hexColor));
    if (getColorIndex >= 0 && getColorIndex < static_cast<int>(colorArray.size()))
        colorIndex = static_cast<unsigned int>(getColorIndex);

    const unsigned int index = colorIndex;
    colorIndex = colorIndex + 1 == colorArray.size() ? 0 : colorIndex + 1;
    return static_cast<Color_t>(TColor::GetColor(colorArray[index].c_str()));
}

std::string Trim(const std::string &text)
{
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return {};
    const auto last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

std::string Upper(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char character) { return std::toupper(character); });
    return text;
}

std::string AtLine(const std::string &path, std::size_t line, const std::string &message)
{
    return path + ":" + std::to_string(line) + ": " + message;
}

template <typename Value>
void ReadRequired(std::istringstream &stream,
                  Value              &value,
                  const std::string  &path,
                  std::size_t         line,
                  const std::string  &description)
{
    if (!(stream >> value))
        throw ConfigurationError(AtLine(path, line, "expected " + description));
}

void RejectTrailing(std::istringstream &stream, const std::string &path, std::size_t line)
{
    std::string trailing;
    if (stream >> trailing)
        throw ConfigurationError(AtLine(path, line, "unexpected value '" + trailing + "'"));
}
} // namespace

EdECalculatorOscar::EdECalculatorOscar(const ReactionConf &reactionConfValue,
                                       const TargetConf   &targetConfValue,
                                       const ReplayConf   &replayConfValue,
                                       const OscarConf    &oscarConfValue)
    : reactionConf(reactionConfValue), targetConf(targetConfValue), replayConf(replayConfValue),
      oscarConf(oscarConfValue)
{
}

EdECalculatorOscar::~EdECalculatorOscar() = default;

void EdECalculatorOscar::LoadReactionFile(const std::string &path)
{
    std::ifstream input(path);
    if (!input.is_open())
        throw ConfigurationError("Unable to open OSCAR E-dE reaction file: " + path);

    std::vector<EdEOscarReaction> parsed;
    EdEOscarReaction              current;
    bool                          inReaction{false};
    bool                          mechanismSet{false};
    std::size_t                   lineNumber{0};
    std::string                   line;

    auto finishReaction = [&]() {
        if (!inReaction)
            throw ConfigurationError(AtLine(path, lineNumber, "END_REACTION without REACTION"));
        if (!mechanismSet)
            throw ConfigurationError(
                AtLine(path, lineNumber, "reaction requires TRANSFER, DETECTED, or ELASTIC"));
        ValidateReaction(current);
        const auto duplicate =
            std::find_if(parsed.begin(), parsed.end(), [&](const EdEOscarReaction &reaction) {
                return reaction.name == current.name;
            });
        if (duplicate != parsed.end())
            throw ConfigurationError(
                AtLine(path, lineNumber, "duplicate reaction name '" + current.name + "'"));
        parsed.emplace_back(std::move(current));
        current      = EdEOscarReaction{};
        inReaction   = false;
        mechanismSet = false;
    };

    while (std::getline(input, line))
    {
        ++lineNumber;
        const auto comment = line.find('#');
        if (comment != std::string::npos)
            line.erase(comment);
        line = Trim(line);
        if (line.empty())
            continue;

        std::istringstream stream(line);
        std::string        keyword;
        stream >> keyword;
        keyword = Upper(keyword);

        if (keyword == "REACTION")
        {
            if (inReaction)
                throw ConfigurationError(
                    AtLine(path, lineNumber, "nested REACTION before END_REACTION"));
            ReadRequired(stream, current.name, path, lineNumber, "a reaction name");
            RejectTrailing(stream, path, lineNumber);
            inReaction = true;
            continue;
        }
        if (keyword == "END_REACTION" || keyword == "END")
        {
            RejectTrailing(stream, path, lineNumber);
            finishReaction();
            continue;
        }
        if (!inReaction)
            throw ConfigurationError(
                AtLine(path, lineNumber, "directive outside a REACTION block"));

        if (keyword == "LAYER")
        {
            std::string value;
            ReadRequired(stream, value, path, lineNumber, "TARGET or BACKING");
            value = Upper(value);
            if (value == "TARGET")
                current.layer = EdEReactionLayer::TARGET;
            else if (value == "BACKING" || value == "DEGRADER")
                current.layer = EdEReactionLayer::BACKING;
            else
                throw ConfigurationError(
                    AtLine(path, lineNumber, "unknown reaction layer '" + value + "'"));
        }
        else if (keyword == "TARGET")
        {
            ReadRequired(stream, current.target.A, path, lineNumber, "target mass number A");
            ReadRequired(stream, current.target.Z, path, lineNumber, "target charge Z");
        }
        else if (keyword == "TRANSFER")
        {
            if (mechanismSet)
                throw ConfigurationError(
                    AtLine(path, lineNumber, "reaction mechanism is already defined"));
            ReadRequired(stream, current.transferredA, path, lineNumber, "transferred mass A");
            ReadRequired(stream, current.transferredZ, path, lineNumber, "transferred charge Z");
            current.mechanism   = EdEReactionMechanism::TRANSFER;
            mechanismSet        = true;
        }
        else if (keyword == "DETECTED")
        {
            if (mechanismSet)
                throw ConfigurationError(
                    AtLine(path, lineNumber, "reaction mechanism is already defined"));
            ReadRequired(stream, current.detected.A, path, lineNumber, "detected mass A");
            ReadRequired(stream, current.detected.Z, path, lineNumber, "detected charge Z");
            current.mechanism   = EdEReactionMechanism::BINARY;
            mechanismSet        = true;
        }
        else if (keyword == "ELASTIC")
        {
            if (mechanismSet)
                throw ConfigurationError(
                    AtLine(path, lineNumber, "reaction mechanism is already defined"));
            current.mechanism = EdEReactionMechanism::ELASTIC;
            mechanismSet      = true;
        }
        else if (keyword == "GRID")
        {
            ReadRequired(stream, current.grid.minimum, path, lineNumber, "minimum excitation");
            ReadRequired(stream, current.grid.maximum, path, lineNumber, "maximum excitation");
            ReadRequired(stream, current.grid.step, path, lineNumber, "excitation step");
            current.hasGrid = true;
        }
        else if (keyword == "LEVELS")
        {
            const auto  previousSize = current.knownLevels.size();
            std::string token;
            while (stream >> token)
            {
                std::size_t consumed{0};
                double      level{0};
                try
                {
                    level = std::stod(token, &consumed);
                }
                catch (const std::exception &)
                {
                    throw ConfigurationError(
                        AtLine(path, lineNumber, "invalid excitation level '" + token + "'"));
                }
                if (consumed != token.size())
                    throw ConfigurationError(
                        AtLine(path, lineNumber, "invalid excitation level '" + token + "'"));
                current.knownLevels.emplace_back(level);
            }
            if (current.knownLevels.size() == previousSize)
                throw ConfigurationError(AtLine(path, lineNumber, "LEVELS requires a value"));
            continue;
        }
        else if (keyword == "BRANCH")
        {
            std::string value;
            ReadRequired(stream, value, path, lineNumber, "HIGH, LOW, or BOTH");
            value = Upper(value);
            if (value == "HIGH")
                current.branch = EdEBranchSelection::HIGH_ENERGY;
            else if (value == "LOW")
                current.branch = EdEBranchSelection::LOW_ENERGY;
            else if (value == "BOTH")
                current.branch = EdEBranchSelection::BOTH;
            else
                throw ConfigurationError(
                    AtLine(path, lineNumber, "unknown kinematic branch '" + value + "'"));
        }
        else if (keyword == "RESOLUTION")
        {
            ReadRequired(stream, current.energyResolution, path, lineNumber,
                         "E resolution in MeV");
            ReadRequired(stream, current.deltaEResolution, path, lineNumber,
                         "dE resolution in MeV");
        }
        else if (keyword == "LINE_COLOR")
            ReadRequired(stream, current.lineColor, path, lineNumber, "ROOT line color");
        else if (keyword == "LINE_STYLE")
            ReadRequired(stream, current.lineStyle, path, lineNumber, "ROOT line style");
        else if (keyword == "LINE_WIDTH")
            ReadRequired(stream, current.lineWidth, path, lineNumber, "ROOT line width");
        else if (keyword == "MARKER_STYLE")
            ReadRequired(stream, current.markerStyle, path, lineNumber, "ROOT marker style");
        else if (keyword == "MARKER_SIZE")
            ReadRequired(stream, current.markerSize, path, lineNumber, "ROOT marker size");
        else
            throw ConfigurationError(
                AtLine(path, lineNumber, "unknown directive '" + keyword + "'"));

        RejectTrailing(stream, path, lineNumber);
    }

    if (inReaction)
        throw ConfigurationError(path + ": unterminated REACTION block for '" + current.name + "'");
    if (parsed.empty())
        throw ConfigurationError("No OSCAR E-dE reactions defined in " + path);

    reactions = std::move(parsed);
    calculators.clear();
    graphs.clear();
    graphsBuilt = false;
}

void EdECalculatorOscar::AddReaction(EdEOscarReaction reaction)
{
    ValidateReaction(reaction);
    const auto duplicate =
        std::find_if(reactions.begin(), reactions.end(), [&](const EdEOscarReaction &existing) {
            return existing.name == reaction.name;
        });
    if (duplicate != reactions.end())
        throw ConfigurationError("Duplicate OSCAR E-dE reaction name: " + reaction.name);
    reactions.emplace_back(std::move(reaction));
    calculators.clear();
    graphs.clear();
    graphsBuilt = false;
}

void EdECalculatorOscar::BuildMaterialStack()
{
    materialStack.clear();
    hasBacking = targetConf.degPresent;

    if (targetConf.material.empty() || Upper(targetConf.material) == "NONE")
        throw ConfigurationError(
            "TARGET_CONF MATERIAL must identify the target material for OSCAR E-dE calculations");

    EdECalculator::EdEMaterialLayer target{targetConf.material, targetConf.thickness,
                                           targetConf.targetNorm};
    if (!hasBacking)
    {
        targetLayerIndex = 0;
        materialStack.emplace_back(std::move(target));
        return;
    }

    if (targetConf.degMaterial.empty() || Upper(targetConf.degMaterial) == "NONE")
        throw ConfigurationError(
            "TARGET_CONF DEG_MATERIAL must identify the degrader material when DEG_PRESENT is YES");

    EdECalculator::EdEMaterialLayer backing{targetConf.degMaterial, targetConf.degThickness,
                                            targetConf.targetNorm};
    if (targetConf.degPos == TargetConf::DegPos::BEFORE)
    {
        backingLayerIndex = 0;
        targetLayerIndex  = 1;
        materialStack.emplace_back(std::move(backing));
        materialStack.emplace_back(std::move(target));
        return;
    }

    targetLayerIndex  = 0;
    backingLayerIndex = 1;
    materialStack.emplace_back(std::move(target));
    materialStack.emplace_back(std::move(backing));
}

void EdECalculatorOscar::BuildDetectorDefinitions()
{
    detectors.clear();
    const auto telescopes = oscarConf.getOscarTelescopes();
    detectors.reserve(telescopes.size());

    for (const auto *telescope : telescopes)
    {
        EdECalculator::EdEDetectorDefinition detector;
        detector.id   = telescope->telescopeID;
        detector.name = telescope->name;

        TVector3 position(telescope->pos_x + oscarConf.xShift, telescope->pos_y + oscarConf.yShift,
                          telescope->pos_z + oscarConf.zShift);
        TVector3 normal(telescope->norm_x, telescope->norm_y, telescope->norm_z);
        for (TVector3 *vector : {&position, &normal})
        {
            vector->RotateZ(oscarConf.phi);
            vector->RotateX(oscarConf.thetaX);
            vector->RotateY(oscarConf.thetaY);
            vector->RotateY(oscarConf.angle);
        }
        if (position.Mag2() <= 0 || normal.Mag2() <= 0)
            throw ConfigurationError("Invalid geometry for OSCAR telescope " +
                                     std::to_string(telescope->telescopeID));
        detector.direction = position.Unit();
        detector.normal    = normal.Unit();

        auto addLayer = [&](const std::string &name, const std::string &material, double thickness,
                            EdECalculator::ThicknessType     thicknessType,
                            EdECalculator::DetectorLayerRole role) {
            if (thickness > 0)
                detector.layers.push_back({name, material, thickness, thicknessType, role});
        };

        addLayer("external dead layer", oscarConf.deadlayerMaterial, oscarConf.deadlayerThickness,
                 EdECalculator::ThicknessType::AREAL_DENSITY,
                 EdECalculator::DetectorLayerRole::PASSIVE);
        addLayer("dE entrance dead layer", oscarConf.edeActiveMaterial, telescope->thick_dead_dE,
                 EdECalculator::ThicknessType::LENGTH, EdECalculator::DetectorLayerRole::PASSIVE);
        addLayer("dE active layer", oscarConf.edeActiveMaterial, telescope->thick_active_dE,
                 EdECalculator::ThicknessType::LENGTH, EdECalculator::DetectorLayerRole::DELTA_E);
        addLayer("E entrance dead layer", oscarConf.edeActiveMaterial, telescope->thick_dead_Eres,
                 EdECalculator::ThicknessType::LENGTH, EdECalculator::DetectorLayerRole::PASSIVE);
        addLayer("E active layer", oscarConf.edeActiveMaterial, telescope->thick_active_Eres,
                 EdECalculator::ThicknessType::LENGTH,
                 EdECalculator::DetectorLayerRole::RESIDUAL_E);
        detectors.emplace_back(std::move(detector));
    }

    if (detectors.empty())
        throw ConfigurationError("No active OSCAR telescopes are available for E-dE calculation");
}

void EdECalculatorOscar::ValidateReaction(const EdEOscarReaction &reaction) const
{
    if (reaction.name.empty())
        throw ConfigurationError("OSCAR E-dE reaction has an empty name");
    if (reaction.target.A == 0 || reaction.target.Z > reaction.target.A)
        throw ConfigurationError("Reaction '" + reaction.name + "' has an invalid target ion");
    if (reaction.mechanism == EdEReactionMechanism::TRANSFER)
    {
        if (reaction.transferredA == 0 || reaction.transferredZ > reaction.transferredA)
            throw ConfigurationError("Reaction '" + reaction.name + "' has an invalid transfer");
    }
    else if (reaction.mechanism == EdEReactionMechanism::BINARY &&
             (reaction.detected.A == 0 || reaction.detected.Z > reaction.detected.A))
        throw ConfigurationError("Reaction '" + reaction.name + "' has an invalid detected ion");
    if (reaction.mechanism == EdEReactionMechanism::ELASTIC &&
        (reaction.hasGrid || !reaction.knownLevels.empty()))
        throw ConfigurationError("Elastic reaction '" + reaction.name +
                                 "' must not define GRID or LEVELS");
    if (reaction.mechanism != EdEReactionMechanism::ELASTIC && !reaction.hasGrid &&
        reaction.knownLevels.empty())
        throw ConfigurationError("Reaction '" + reaction.name + "' requires GRID and/or LEVELS");
    if (reaction.hasGrid &&
        (!std::isfinite(reaction.grid.minimum) || !std::isfinite(reaction.grid.maximum) ||
         !std::isfinite(reaction.grid.step) || reaction.grid.minimum < 0 ||
         reaction.grid.maximum < reaction.grid.minimum || reaction.grid.step <= 0))
        throw ConfigurationError("Reaction '" + reaction.name + "' has an invalid GRID");
    if (std::any_of(reaction.knownLevels.begin(), reaction.knownLevels.end(),
                    [](double level) { return !std::isfinite(level) || level < 0; }))
        throw ConfigurationError("Reaction '" + reaction.name + "' has an invalid LEVELS value");
    if (!std::isfinite(reaction.energyResolution) || reaction.energyResolution < 0 ||
        !std::isfinite(reaction.deltaEResolution) || reaction.deltaEResolution < 0)
        throw ConfigurationError("Reaction '" + reaction.name + "' has an invalid RESOLUTION");
    if (reaction.lineWidth <= 0 || reaction.markerSize < 0)
        throw ConfigurationError("Reaction '" + reaction.name + "' has invalid graph styling");
}

std::size_t EdECalculatorOscar::ResolveReactionLayer(EdEReactionLayer layer) const
{
    if (layer == EdEReactionLayer::TARGET)
        return targetLayerIndex;
    if (!hasBacking)
        throw ConfigurationError("An OSCAR E-dE reaction requests BACKING, but DEG_PRESENT is NO");
    return backingLayerIndex;
}

void EdECalculatorOscar::BuildGraphs()
{
    graphsBuilt = false;
    if (reactions.empty())
        throw ConfigurationError("No OSCAR E-dE reactions have been configured");

    BuildMaterialStack();
    BuildDetectorDefinitions();
    calculators.clear();
    graphs.clear();

    EdECalculator::EdESettings settings;
    settings.nuclearDataFile     = replayConf.conf_path + "/NuclearData/nutab12";
    settings.energyLossDirectory = replayConf.conf_path + "/EnergyLoss";
    settings.energyLossTableType = EnergyLossTableType();

    for (std::size_t reactionIndex = 0; reactionIndex < reactions.size(); ++reactionIndex)
    {
        auto reaction      = reactions[reactionIndex];
        reaction.lineColor = ReactionColor(reaction, reactionIndex);
        ValidateReaction(reaction);

        // A shared reaction file may describe both target and backing channels.
        // Backing reactions are inapplicable, rather than malformed, when the
        // selector explicitly says that no degrader/backing is installed.
        if (reaction.layer == EdEReactionLayer::BACKING && !hasBacking)
            continue;

        const auto layerIndex = ResolveReactionLayer(reaction.layer);
        if (reaction.mechanism == EdEReactionMechanism::ELASTIC)
        {
            const std::string configuredTarget = std::to_string(reaction.target.A) + "-" +
                                                 std::to_string(reaction.target.Z);
            const std::string &layerMaterial = reaction.layer == EdEReactionLayer::TARGET
                                                   ? targetConf.material
                                                   : targetConf.degMaterial;
            if (configuredTarget != layerMaterial)
                throw ConfigurationError("Elastic reaction '" + reaction.name + "' targets " +
                                         configuredTarget + ", but its selected layer is " +
                                         layerMaterial);
        }
        std::vector<double> grid;
        if (reaction.hasGrid)
            grid = EdECalculator::MakeExcitationGrid(reaction.grid.minimum * UNITS::MeV,
                                                     reaction.grid.maximum * UNITS::MeV,
                                                     reaction.grid.step * UNITS::MeV);
        std::vector<double> levels;
        levels.reserve(reaction.knownLevels.size());
        for (const double level : reaction.knownLevels)
            levels.emplace_back(level * UNITS::MeV);
        std::sort(levels.begin(), levels.end());
        levels.erase(std::unique(levels.begin(), levels.end()), levels.end());

        if (reaction.mechanism == EdEReactionMechanism::ELASTIC)
        {
            grid.clear();
            levels.assign(1, 0.);
        }

        std::vector<EdECalculator::KinematicBranch> branches;
        if (reaction.branch != EdEBranchSelection::LOW_ENERGY)
            branches.emplace_back(EdECalculator::KinematicBranch::HIGH_ENERGY);
        if (reaction.branch != EdEBranchSelection::HIGH_ENERGY)
            branches.emplace_back(EdECalculator::KinematicBranch::LOW_ENERGY);

        for (const auto branch : branches)
        {
            EdECalculator::EdEReactionDefinition definition;
            if (reaction.mechanism == EdEReactionMechanism::TRANSFER)
                definition = EdECalculator::MakeTransferReaction(
                    reaction.name, reactionConf.beam, reaction.target, reaction.transferredA,
                    reaction.transferredZ, reactionConf.energy, layerIndex, 0.5, branch);
            else
            {
                const auto detected = reaction.mechanism == EdEReactionMechanism::ELASTIC
                                          ? reactionConf.beam
                                          : reaction.detected;
                definition = EdECalculator::MakeBinaryReaction(
                    reaction.name, reactionConf.beam, reaction.target, detected,
                    reactionConf.energy, layerIndex, 0.5, branch);
            }

            auto calculator = std::make_unique<EdECalculator>(settings, std::move(definition));
            calculator->SetMaterialStack(materialStack);
            calculator->Validate(detectors);

            const bool        showBranch = reaction.branch == EdEBranchSelection::BOTH;
            const bool        highBranch = branch == EdECalculator::KinematicBranch::HIGH_ENERGY;
            const std::string branchSuffix = showBranch ? (highBranch ? "_high" : "_low") : "";
            const std::string legendSuffix = showBranch ? (highBranch ? " (high)" : " (low)") : "";

            for (const auto &detector : detectors)
            {
                if (!grid.empty())
                {
                    BuildGraph(reaction, *calculator, detector, grid, EdEGraphMode::SMOOTH_GRID,
                               EdEGraphAxes::REACTION_ENERGY_DELTA_E, branchSuffix, legendSuffix);
                    BuildGraph(reaction, *calculator, detector, grid, EdEGraphMode::SMOOTH_GRID,
                               EdEGraphAxes::RESIDUAL_E_DELTA_E, branchSuffix, legendSuffix);
                }
                if (!levels.empty())
                {
                    const auto mode = reaction.mechanism == EdEReactionMechanism::ELASTIC
                                          ? EdEGraphMode::ELASTIC_REGION
                                          : EdEGraphMode::KNOWN_LEVELS;
                    BuildGraph(reaction, *calculator, detector, levels, mode,
                               EdEGraphAxes::REACTION_ENERGY_DELTA_E, branchSuffix, legendSuffix);
                    BuildGraph(reaction, *calculator, detector, levels, mode,
                               EdEGraphAxes::RESIDUAL_E_DELTA_E, branchSuffix, legendSuffix);
                }
            }
            calculators.emplace_back(std::move(calculator));
        }
    }

    if (graphs.empty())
        throw ConfigurationError(
            "No physical OSCAR E-dE points were produced for any configured reaction");
    graphsBuilt = true;
}

void EdECalculatorOscar::BuildGraph(const EdEOscarReaction                     &reaction,
                                    EdECalculator                              &calculator,
                                    const EdECalculator::EdEDetectorDefinition &detector,
                                    const std::vector<double>                  &excitationEnergies,
                                    EdEGraphMode                                mode,
                                    EdEGraphAxes                                axes,
                                    const std::string                          &branchSuffix,
                                    const std::string                          &legendSuffix)
{
    auto              graph    = std::make_unique<TGraphAsymmErrors>();
    const std::string modeName = mode == EdEGraphMode::SMOOTH_GRID
                                     ? "grid"
                                     : mode == EdEGraphMode::ELASTIC_REGION ? "elastic" : "levels";
    const std::string axesName =
        axes == EdEGraphAxes::REACTION_ENERGY_DELTA_E ? "Etot_dE" : "E_dE";
    const std::string name = "g_EdE_" + SafeName(reaction.name) + "_tel_" +
                             std::to_string(detector.id) + "_" + modeName + "_" + axesName +
                             branchSuffix;
    graph->SetName(name.c_str());
    const std::string xTitle = axes == EdEGraphAxes::REACTION_ENERGY_DELTA_E
                                   ? "Particle energy at reaction [MeV]"
                                   : "Residual E deposit [MeV]";
    graph->SetTitle((reaction.name + legendSuffix + " - OSCAR telescope " +
                     std::to_string(detector.id) + ";" + xTitle + ";Delta E [MeV]")
                        .c_str());
    graph->SetLineColor(reaction.lineColor);
    graph->SetLineStyle(reaction.lineStyle);
    graph->SetLineWidth(reaction.lineWidth);
    graph->SetFillColor(reaction.lineColor);
    graph->SetFillStyle(3005);
    graph->SetMarkerColor(reaction.lineColor);
    graph->SetMarkerStyle(reaction.markerStyle);
    graph->SetMarkerSize(reaction.markerSize);

    const auto points = calculator.Calculate(detector, excitationEnergies);
    for (const auto &point : points)
    {
        if (!point.valid)
            continue;
        const bool   reactionEnergy = axes == EdEGraphAxes::REACTION_ENERGY_DELTA_E;
        const auto  &xError = reactionEnergy ? point.energyAtReactionError : point.residualEError;
        const double x =
            reactionEnergy ? point.center.energyAtReaction : point.center.residualEDeposit;
        const int index = graph->GetN();
        graph->SetPoint(index, x / UNITS::MeV, point.center.deltaEDeposit / UNITS::MeV);
        const double energyResolution = reaction.energyResolution * UNITS::MeV;
        const double deltaEResolution = reaction.deltaEResolution * UNITS::MeV;
        graph->SetPointError(
            index, std::hypot(xError.lower, energyResolution) / UNITS::MeV,
            std::hypot(xError.upper, energyResolution) / UNITS::MeV,
            std::hypot(point.deltaEError.lower, deltaEResolution) / UNITS::MeV,
            std::hypot(point.deltaEError.upper, deltaEResolution) / UNITS::MeV);
    }

    if (graph->GetN() > 0)
        graphs.push_back(
            {reaction.name + legendSuffix, detector.id, mode, axes, std::move(graph)});
}

void EdECalculatorOscar::WriteCanvases(
    TDirectory                                     &reactionEnergyParent,
    const std::unordered_map<unsigned int, TH2F *> &reactionEnergyMatrices,
    TDirectory                                     &residualEnergyParent,
    const std::unordered_map<unsigned int, TH2F *> &residualEnergyMatrices) const
{
    if (!graphsBuilt)
        throw ConfigurationError("OSCAR E-dE graphs must be built before canvases are written");

    WriteCanvasFamily(reactionEnergyParent, reactionEnergyMatrices,
                      EdEGraphAxes::REACTION_ENERGY_DELTA_E);
    WriteCanvasFamily(residualEnergyParent, residualEnergyMatrices,
                      EdEGraphAxes::RESIDUAL_E_DELTA_E);
}

void EdECalculatorOscar::WriteCanvases(TFile &file, const std::string &oscarDirectory) const
{
    auto *reactionEnergyDirectory =
        file.GetDirectory((oscarDirectory + "/Etot_dE").c_str());
    auto *residualEnergyDirectory = file.GetDirectory((oscarDirectory + "/EdE").c_str());
    if (!reactionEnergyDirectory || !residualEnergyDirectory)
        throw ConfigurationError("Unable to find E-dE matrix directories below '" +
                                 oscarDirectory + "' in " + file.GetName());

    std::unordered_map<unsigned int, TH2F *> reactionEnergyMatrices;
    std::unordered_map<unsigned int, TH2F *> residualEnergyMatrices;
    for (const auto &detector : detectors)
    {
        const std::string reactionEnergyName =
            "m_Etot_dE_tel_" + std::to_string(detector.id);
        const std::string residualEnergyName = "m_EdE_tel_" + std::to_string(detector.id);
        if (auto *matrix = dynamic_cast<TH2F *>(
                reactionEnergyDirectory->Get(reactionEnergyName.c_str())))
            reactionEnergyMatrices.emplace(detector.id, matrix);
        if (auto *matrix =
                dynamic_cast<TH2F *>(residualEnergyDirectory->Get(residualEnergyName.c_str())))
            residualEnergyMatrices.emplace(detector.id, matrix);
    }

    reactionEnergyDirectory->Delete("KinematicLines;*");
    residualEnergyDirectory->Delete("KinematicLines;*");
    WriteCanvases(*reactionEnergyDirectory, reactionEnergyMatrices, *residualEnergyDirectory,
                  residualEnergyMatrices);
}

void EdECalculatorOscar::WriteCanvasFamily(TDirectory                                     &parent,
                                           const std::unordered_map<unsigned int, TH2F *> &matrices,
                                           EdEGraphAxes axes) const
{
    TDirectory::TContext context(&parent);
    const bool           reactionEnergy = axes == EdEGraphAxes::REACTION_ENERGY_DELTA_E;
    for (const auto &matrixEntry : matrices)
    {
        TH2F *matrix = matrixEntry.second;
        if (!matrix)
            continue;

        const std::string axesName = reactionEnergy ? "Etot_dE" : "EdE";
        const std::string name     = "c_" + axesName + "_tel_" + std::to_string(matrixEntry.first);
        const std::string title =
            "OSCAR " + axesName + " overlay - telescope " + std::to_string(matrixEntry.first);
        TCanvas canvas(name.c_str(), title.c_str(), 1000, 800);
        canvas.cd();
        matrix->Draw("COLZ");

        TLegend                         legend(0.58, 0.68, 0.88, 0.88);
        std::unordered_set<std::string> legendReactions;
        legend.SetBorderSize(0);
        legend.SetFillStyle(0);
        for (const auto &entry : graphs)
        {
            if (entry.telescopeID != matrixEntry.first || entry.axes != axes)
                continue;

            const bool grid    = entry.mode == EdEGraphMode::SMOOTH_GRID;
            const bool elastic = entry.mode == EdEGraphMode::ELASTIC_REGION;
            // The matrix already supplies the axes. ROOT's `A4` clears those axes and
            // every earlier primitive, so `4 SAME` is its overlay-safe equivalent.
            entry.graph->Draw(grid ? "4 SAME" : elastic ? "2 SAME" : "PZ SAME");
            if (legendReactions.emplace(entry.reactionName).second)
                legend.AddEntry(entry.graph.get(), entry.reactionName.c_str(),
                                (grid || elastic) ? "f" : "p");
        }
        if (!legendReactions.empty())
            legend.Draw();
        canvas.Modified();
        canvas.Update();
        canvas.Write(canvas.GetName(), TObject::kOverwrite);
    }
}

int EdECalculatorOscar::ReactionColor(const EdEOscarReaction &reaction, std::size_t index)
{
    if (reaction.lineColor != 0)
        return reaction.lineColor;

    return GetColor(static_cast<int>(index));
}

std::string EdECalculatorOscar::EnergyLossTableType() const
{
    switch (targetConf.elossTableType)
    {
    case 0:
        return "SRIM";
    case 1:
        return "G4Table";
    case 2:
        return "LISE";
    default:
        throw ConfigurationError("Unknown target ELOSS_TABLE_TYPE " +
                                 std::to_string(targetConf.elossTableType));
    }
}

std::string EdECalculatorOscar::SafeName(const std::string &name)
{
    std::string result;
    result.reserve(name.size());
    for (const unsigned char character : name)
        result.push_back(std::isalnum(character) || character == '_' ? character : '_');
    return result.empty() ? "reaction" : result;
}
