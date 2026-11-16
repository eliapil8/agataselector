#include "EdECalculator.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <set>
#include <sstream>
#include <utility>

namespace
{
constexpr double directionTolerance = 1.e-12;

bool IsIonValid(const EdECalculator::Ion &ion)
{
    return ion.Z > 0 && ion.Z <= ion.A;
}

/**
 * Return the unsigned incidence angle between a trajectory and a plane normal.
 *
 * Angles greater than pi/2 are folded to pi - angle, producing a result in
 * [0, pi/2]. A planar material layer has the same effective path length when
 * crossed from either side, so directions at theta and pi - theta must give
 * the same energy loss. Traversal direction is determined separately from the
 * signed direction-normal dot product.
 *
 * @param first Particle trajectory direction.
 * @param second Surface-normal direction of the material layer.
 * @return Acute incidence angle in radians.
 */
double FoldedAngle(const TVector3 &first, const TVector3 &second)
{
    double angle = first.Angle(second);
    if (angle > UNITS::CONSTANTS::halfpi)
        angle = UNITS::CONSTANTS::pi - angle;
    return angle;
}

std::string JoinDiagnostics(const std::vector<std::string> &diagnostics)
{
    std::ostringstream message;
    for (const auto &diagnostic : diagnostics)
        message << "\n  - " << diagnostic;
    return message.str();
}
} // namespace

EdECalculator::EdECalculator(EdESettings settingsIn, EdEReactionDefinition reactionIn)
    : settings(std::move(settingsIn)), reaction(std::move(reactionIn))
{
    ValidateReactionStructure(reaction);
}

EdECalculator::~EdECalculator() = default;

bool EdECalculator::EnergyLossKey::operator<(const EnergyLossKey &other) const
{

    if (ion.A != other.ion.A)
        return ion.A < other.ion.A;
    if (ion.Z != other.ion.Z)
        return ion.Z < other.ion.Z;
    return material < other.material;
}

void EdECalculator::SetMaterialStack(std::vector<EdEMaterialLayer> layers)
{
    materialStack = std::move(layers);
}

EdECalculator::EdEReactionDefinition EdECalculator::MakeBinaryReaction(
    const std::string &name,
    const Ion         &beam,
    const Ion         &target,
    const Ion         &detected,
    double             entranceBeamEnergy,
    std::size_t        reactionLayerIndex,
    double             nominalDepth,
    KinematicBranch    branch)
{
    if (!IsIonValid(beam) || !IsIonValid(target) || !IsIonValid(detected))
        throw EdEConfigurationError("Cannot construct reaction '" + name +
                                    "': beam, target, or detected ion is invalid");
    if (detected.A > beam.A + target.A || detected.Z > beam.Z + target.Z)
        throw EdEConfigurationError("Cannot construct reaction '" + name +
                                    "': detected fragment exceeds the compound system");

    EdEReactionDefinition reaction;
    reaction.name     = name;
    reaction.beam     = beam;
    reaction.target   = target;
    reaction.detected = detected;
    reaction.residual = Ion{beam.A + target.A - detected.A, beam.Z + target.Z - detected.Z};
    reaction.entranceBeamEnergy = entranceBeamEnergy;
    reaction.reactionLayerIndex = reactionLayerIndex;
    reaction.nominalDepth       = nominalDepth;
    reaction.branch             = branch;
    ValidateReactionStructure(reaction);
    return reaction;
}

EdECalculator::EdEReactionDefinition EdECalculator::MakeTransferReaction(
    const std::string &name,
    const Ion         &beam,
    const Ion         &target,
    unsigned int       transferredA,
    unsigned int       transferredZ,
    double             entranceBeamEnergy,
    std::size_t        reactionLayerIndex,
    double             nominalDepth,
    KinematicBranch    branch)
{
    if (transferredA == 0 || transferredZ > transferredA || transferredA >= beam.A ||
        transferredZ > beam.Z)
    {
        throw EdEConfigurationError(
            "Cannot construct transfer reaction '" + name + "': invalid transferred (A,Z) = (" +
            std::to_string(transferredA) + "," + std::to_string(transferredZ) + ")");
    }

    const Ion detected{beam.A - transferredA, beam.Z - transferredZ};
    auto      reaction = MakeBinaryReaction(name, beam, target, detected, entranceBeamEnergy,
                                            reactionLayerIndex, nominalDepth, branch);

    const Ion expectedResidual{target.A + transferredA, target.Z + transferredZ};
    if (reaction.residual != expectedResidual)
        throw EdEConfigurationError(
            "Internal conservation error while constructing transfer reaction '" + name + "'");
    return reaction;
}

void EdECalculator::ValidateReactionStructure(const EdEReactionDefinition &reaction)
{
    if (reaction.name.empty())
        throw EdEConfigurationError("E-dE reaction has an empty name");
    if (!IsIonValid(reaction.beam) || !IsIonValid(reaction.target) ||
        !IsIonValid(reaction.detected) || !IsIonValid(reaction.residual))
        throw EdEConfigurationError("Reaction '" + reaction.name + "' contains an invalid ion");
    if (reaction.beam.A + reaction.target.A != reaction.detected.A + reaction.residual.A ||
        reaction.beam.Z + reaction.target.Z != reaction.detected.Z + reaction.residual.Z)
        throw EdEConfigurationError("Reaction '" + reaction.name +
                                    "' does not conserve mass number and charge");
    if (reaction.entranceBeamEnergy <= 0)
        throw EdEConfigurationError("Reaction '" + reaction.name +
                                    "' has a non-positive entrance beam energy");
    if (reaction.beamDirection.Mag2() <= directionTolerance)
        throw EdEConfigurationError("Reaction '" + reaction.name + "' has no beam direction");
    if (reaction.nominalDepth < 0 || reaction.nominalDepth > 1)
        throw EdEConfigurationError("Reaction '" + reaction.name +
                                    "' has a nominal depth outside [0,1]");
}

void EdECalculator::ValidateDetectorStructure(const EdEDetectorDefinition &detector)
{
    if (detector.direction.Mag2() <= directionTolerance)
        throw EdEConfigurationError("E-dE detector '" + detector.name + "' has no direction");
    if (detector.normal.Mag2() <= directionTolerance)
        throw EdEConfigurationError("E-dE detector '" + detector.name + "' has no surface normal");
    if (detector.layers.empty())
        throw EdEConfigurationError("E-dE detector '" + detector.name + "' has no layers");

    unsigned int deltaELayers    = 0;
    unsigned int residualELayers = 0;
    for (const auto &layer : detector.layers)
    {
        if (layer.material.empty())
            throw EdEConfigurationError("Detector '" + detector.name +
                                        "' has a layer without material");
        if (layer.thickness < 0)
            throw EdEConfigurationError("Detector '" + detector.name +
                                        "' has a negative layer thickness");
        if (layer.role == DetectorLayerRole::DELTA_E)
            ++deltaELayers;
        if (layer.role == DetectorLayerRole::RESIDUAL_E)
            ++residualELayers;
    }
    if (deltaELayers != 1 || residualELayers != 1)
        throw EdEConfigurationError("Detector '" + detector.name +
                                    "' must have exactly one active dE and one active E layer");
}

void EdECalculator::ValidateMaterialStack() const
{
    if (materialStack.empty())
        throw EdEConfigurationError("E-dE material stack is empty");
    for (const auto &layer : materialStack)
    {
        if (layer.name.empty())
            throw EdEConfigurationError("E-dE material stack contains an unnamed layer");
        if (layer.arealThickness < 0)
            throw EdEConfigurationError("Material layer '" + layer.name +
                                        "' has negative thickness");
        if (layer.normal.Mag2() <= directionTolerance)
            throw EdEConfigurationError("Material layer '" + layer.name + "' has no normal");
    }
}

std::string EdECalculator::IonName(const Ion &ion)
{
    return std::to_string(ion.A) + "-" + std::to_string(ion.Z);
}

std::string EdECalculator::EnergyLossTablePath(const std::string &material, const Ion &ion) const
{
    std::string directory = settings.energyLossDirectory;
    if (!directory.empty() && directory.back() != '/')
        directory += '/';
    return directory + "Target-" + material + "_Ion-" + IonName(ion) + ".txt";
}

void EdECalculator::Validate(const std::vector<EdEDetectorDefinition> &detectors)
{
    std::vector<std::string> diagnostics;

    try
    {
        ValidateMaterialStack();
    }
    catch (const EdEConfigurationError &error)
    {
        diagnostics.emplace_back(error.what());
    }

    if (settings.nuclearDataFile.empty() || !std::ifstream(settings.nuclearDataFile).good())
        diagnostics.emplace_back("nuclear mass table not found: " + settings.nuclearDataFile);
    if (settings.energyLossDirectory.empty())
        diagnostics.emplace_back("energy-loss table directory is empty");
    if (settings.energyLossSlices <= 0)
        diagnostics.emplace_back("energy-loss slice count must be positive");
    if (detectors.empty())
        diagnostics.emplace_back("no E-dE detectors were supplied");

    for (const auto &detector : detectors)
    {
        try
        {
            ValidateDetectorStructure(detector);
        }
        catch (const EdEConfigurationError &error)
        {
            diagnostics.emplace_back(error.what());
        }
    }

    std::set<EnergyLossKey> requiredTables;
    try
    {
        ValidateReactionStructure(reaction);
        if (reaction.reactionLayerIndex >= materialStack.size())
            throw EdEConfigurationError(
                "Reaction '" + reaction.name + "' refers to material layer " +
                std::to_string(reaction.reactionLayerIndex) + ", but the stack has only " +
                std::to_string(materialStack.size()) + " layers");

        for (std::size_t layer = 0; layer <= reaction.reactionLayerIndex; ++layer)
        {
            if (materialStack[layer].arealThickness > 0)
                requiredTables.insert({materialStack[layer].name, reaction.beam});
        }

        for (const auto &detector : detectors)
        {
            if (detector.direction.Mag2() <= directionTolerance)
                continue;
            const bool forward =
                detector.direction.Dot(materialStack[reaction.reactionLayerIndex].normal) >= 0;
            if (forward)
            {
                for (std::size_t layer = reaction.reactionLayerIndex; layer < materialStack.size();
                     ++layer)
                {
                    if (materialStack[layer].arealThickness > 0)
                        requiredTables.insert({materialStack[layer].name, reaction.detected});
                }
            }
            else
            {
                for (std::size_t layer = 0; layer <= reaction.reactionLayerIndex; ++layer)
                {
                    if (materialStack[layer].arealThickness > 0)
                        requiredTables.insert({materialStack[layer].name, reaction.detected});
                }
            }

            for (const auto &detectorLayer : detector.layers)
            {
                if (detectorLayer.thickness > 0)
                    requiredTables.insert({detectorLayer.material, reaction.detected});
            }
        }

        GetReactionPrototype();
    }
    catch (const std::exception &error)
    {
        diagnostics.emplace_back("reaction '" + reaction.name + "': " + error.what());
    }

    for (const auto &required : requiredTables)
    {
        const std::string path = EnergyLossTablePath(required.material, required.ion);
        if (!std::ifstream(path).good())
        {
            diagnostics.emplace_back("missing stopping table for ion " + IonName(required.ion) +
                                     " in material '" + required.material + "': " + path);
            continue;
        }
        try
        {
            GetEnergyLoss(required.material, required.ion);
        }
        catch (const std::exception &error)
        {
            diagnostics.emplace_back("invalid stopping table '" + path + "': " + error.what());
        }
    }

    if (!diagnostics.empty())
        throw EdEConfigurationError("E-dE calculator configuration is invalid:" +
                                    JoinDiagnostics(diagnostics));
}

EnergyLoss &EdECalculator::GetEnergyLoss(const std::string &material, const Ion &ion)
{
    const EnergyLossKey key{material, ion};
    auto                found = energyLossTables.find(key);
    if (found != energyLossTables.end())
        return *found->second;

    const std::string path = EnergyLossTablePath(material, ion);
    if (!std::ifstream(path).good())
        throw EdEConfigurationError("Missing stopping table for ion " + IonName(ion) +
                                    " in material '" + material + "': " + path);

    auto table =
        std::make_unique<EnergyLoss>(path, settings.energyLossTableType, settings.energyLossSlices);
    auto inserted = energyLossTables.emplace(key, std::move(table));
    return *inserted.first->second;
}

ReactionReconstruction2body<long double> &EdECalculator::GetReactionPrototype()
{
    if (reactionPrototype)
        return *reactionPrototype;

    using Reconstruction = ReactionReconstruction2body<long double>;
    reactionPrototype    = std::make_unique<Reconstruction>(
        Reconstruction::ReactionInput2body{
            ReactionFragment::FragmentSettings(reaction.beam.A, reaction.beam.Z, 0,
                                                  reaction.entranceBeamEnergy, 0),
            ReactionFragment::FragmentSettings(reaction.target.A, reaction.target.Z, 0, 0, 0),
            ReactionFragment::FragmentSettings(reaction.residual.A, reaction.residual.Z, 0, 0, 0),
            ReactionFragment::FragmentSettings(reaction.detected.A, reaction.detected.Z, 0, 0, 0)},
        settings.nuclearDataFile);
    reactionPrototype->ChooseFixed(4);
    reactionPrototype->ChooseExFixed(4);

    return *reactionPrototype;
}

double EdECalculator::Slow(const std::string &material,
                           const Ion         &ion,
                           double             energy,
                           double             thickness,
                           ThicknessType      thicknessType,
                           double             angle)
{
    if (energy <= 0 || thickness <= 0)
        return std::max(0., energy);

    auto &table = GetEnergyLoss(material, ion);
    if (energy > table.GetMax())
    {
        throw EdEConfigurationError(
            "Stopping table for ion " + IonName(ion) + " in material '" + material + "' ends at " +
            std::to_string(table.GetMax() / UNITS::MeV) + " MeV, below the requested " +
            std::to_string(energy / UNITS::MeV) + " MeV");
    }

    const double result = thicknessType == ThicknessType::AREAL_DENSITY
                              ? table.SlowWithDensity(energy, thickness, angle)
                              : table.Slow(energy, thickness, angle);
    return std::max(0., result);
}

double EdECalculator::BeamEnergyAtDepth(double reactionDepth)
{
    double energy = reaction.entranceBeamEnergy;
    for (std::size_t index = 0; index <= reaction.reactionLayerIndex; ++index)
    {
        const auto  &layer    = materialStack.at(index);
        const double fraction = index == reaction.reactionLayerIndex ? reactionDepth : 1.;
        const double angle    = FoldedAngle(reaction.beamDirection, layer.normal);
        energy = Slow(layer.name, reaction.beam, energy, layer.arealThickness * fraction,
                      ThicknessType::AREAL_DENSITY, angle);
        if (energy <= 0)
            return 0;
    }
    return energy;
}

double EdECalculator::TransportOutOfTarget(const EdEDetectorDefinition &detector,
                                           double                       energy,
                                           double                       reactionDepth,
                                           StopLocation                &stopLocation)
{
    const auto &reactionLayer = materialStack.at(reaction.reactionLayerIndex);
    const bool  forward       = detector.direction.Dot(reactionLayer.normal) >= 0;

    if (forward)
    {
        for (std::size_t index = reaction.reactionLayerIndex; index < materialStack.size(); ++index)
        {
            const auto  &layer    = materialStack[index];
            const double fraction = index == reaction.reactionLayerIndex ? 1. - reactionDepth : 1.;
            energy =
                Slow(layer.name, reaction.detected, energy, layer.arealThickness * fraction,
                     ThicknessType::AREAL_DENSITY, FoldedAngle(detector.direction, layer.normal));
            if (energy <= 0)
            {
                stopLocation = StopLocation::TARGET_STACK;
                return 0;
            }
        }
    }
    else
    {
        for (std::size_t reverse = reaction.reactionLayerIndex + 1; reverse > 0; --reverse)
        {
            const std::size_t index    = reverse - 1;
            const auto       &layer    = materialStack[index];
            const double      fraction = index == reaction.reactionLayerIndex ? reactionDepth : 1.;
            energy =
                Slow(layer.name, reaction.detected, energy, layer.arealThickness * fraction,
                     ThicknessType::AREAL_DENSITY, FoldedAngle(detector.direction, layer.normal));
            if (energy <= 0)
            {
                stopLocation = StopLocation::TARGET_STACK;
                return 0;
            }
        }
    }
    return energy;
}

void EdECalculator::TransportThroughDetector(const EdEDetectorDefinition &detector, EdEPoint &point)
{
    double       energy = point.energyAtDetector;
    const double angle  = FoldedAngle(detector.direction, detector.normal);

    for (const auto &layer : detector.layers)
    {
        if (energy <= 0)
            break;
        const double before  = energy;
        energy               = Slow(layer.material, reaction.detected, energy, layer.thickness,
                                    layer.thicknessType, angle);
        const double deposit = before - energy;

        switch (layer.role)
        {
        case DetectorLayerRole::DELTA_E:
            point.deltaEDeposit += deposit;
            if (energy <= 0)
                point.stopLocation = StopLocation::DELTA_E_LAYER;
            break;
        case DetectorLayerRole::RESIDUAL_E:
            point.residualEDeposit += deposit;
            if (energy <= 0)
                point.stopLocation = StopLocation::RESIDUAL_E_LAYER;
            break;
        case DetectorLayerRole::PASSIVE:
            if (energy <= 0)
                point.stopLocation = StopLocation::PASSIVE_DETECTOR_LAYER;
            break;
        }
    }

    point.totalActiveDeposit = point.deltaEDeposit + point.residualEDeposit;
    point.punchThroughEnergy = energy;
}

EdECalculator::EdEPoint EdECalculator::CalculateAtDepth(const EdEDetectorDefinition &detector,
                                                        double excitationEnergy,
                                                        double reactionDepth)
{
    EdEPoint point;
    point.excitationEnergy = excitationEnergy;
    point.reactionDepth    = reactionDepth;

    if (excitationEnergy < 0)
    {
        point.diagnostic = "Negative excitation energy";
        return point;
    }

    point.beamEnergyAtReaction = BeamEnergyAtDepth(reactionDepth);
    if (point.beamEnergyAtReaction <= 0)
    {
        point.stopLocation = StopLocation::TARGET_STACK;
        point.diagnostic   = "Beam stops before the reaction position";
        return point;
    }

    auto reconstruction = GetReactionPrototype();
    try
    {
        reconstruction.SetBeamEnergy(point.beamEnergyAtReaction);
        reconstruction.GetExFreeFragment().Set_Ex(excitationEnergy);
        const double theta = reaction.beamDirection.Angle(detector.direction);
        reconstruction.Set_Theta(theta, reaction.branch == KinematicBranch::HIGH_ENERGY);
        point.energyAtReaction = static_cast<double>(reconstruction.GetFixedFragment().Get_Ek());
        if (!std::isfinite(point.energyAtReaction) || point.energyAtReaction <= 0)
        {
            point.diagnostic = "No physical detected-fragment energy for this excitation and angle";
            return point;
        }
    }
    catch (const std::exception &error)
    {
        point.diagnostic = error.what();
        return point;
    }

    point.energyAtDetector =
        TransportOutOfTarget(detector, point.energyAtReaction, reactionDepth, point.stopLocation);
    if (point.energyAtDetector <= 0)
    {
        point.diagnostic = "Detected fragment stops in the target/backing stack";
        return point;
    }

    TransportThroughDetector(detector, point);
    point.valid = true;
    return point;
}

EdECalculator::EdEAsymmetricError EdECalculator::MakeError(double center, double front, double back)
{
    const double minimum = std::min(center, std::min(front, back));
    const double maximum = std::max(center, std::max(front, back));
    return {center - minimum, maximum - center};
}

EdECalculator::EdEDepthEnvelope EdECalculator::Calculate(const EdEDetectorDefinition &detector,
                                                         double excitationEnergy)
{
    ValidateMaterialStack();
    ValidateDetectorStructure(detector);
    if (reaction.reactionLayerIndex >= materialStack.size())
        throw EdEConfigurationError("Reaction '" + reaction.name +
                                    "' refers to a non-existent material layer");

    EdEDepthEnvelope envelope;
    envelope.front  = CalculateAtDepth(detector, excitationEnergy, 0.);
    envelope.center = CalculateAtDepth(detector, excitationEnergy, reaction.nominalDepth);
    envelope.back   = CalculateAtDepth(detector, excitationEnergy, 1.);
    envelope.valid  = envelope.front.valid && envelope.center.valid && envelope.back.valid;

    if (envelope.valid)
    {
        envelope.beamEnergyAtReactionError =
            MakeError(envelope.center.beamEnergyAtReaction, envelope.front.beamEnergyAtReaction,
                      envelope.back.beamEnergyAtReaction);
        envelope.energyAtReactionError =
            MakeError(envelope.center.energyAtReaction, envelope.front.energyAtReaction,
                      envelope.back.energyAtReaction);
        envelope.energyAtDetectorError =
            MakeError(envelope.center.energyAtDetector, envelope.front.energyAtDetector,
                      envelope.back.energyAtDetector);
        envelope.deltaEError = MakeError(envelope.center.deltaEDeposit,
                                         envelope.front.deltaEDeposit, envelope.back.deltaEDeposit);
        envelope.residualEError =
            MakeError(envelope.center.residualEDeposit, envelope.front.residualEDeposit,
                      envelope.back.residualEDeposit);
        envelope.totalActiveDepositError =
            MakeError(envelope.center.totalActiveDeposit, envelope.front.totalActiveDeposit,
                      envelope.back.totalActiveDeposit);
    }
    else
    {
        std::vector<std::string> diagnostics;
        if (!envelope.front.valid)
            diagnostics.emplace_back("front: " + envelope.front.diagnostic);
        if (!envelope.center.valid)
            diagnostics.emplace_back("center: " + envelope.center.diagnostic);
        if (!envelope.back.valid)
            diagnostics.emplace_back("back: " + envelope.back.diagnostic);
        envelope.diagnostic = "Incomplete reaction-depth envelope:" + JoinDiagnostics(diagnostics);
    }
    return envelope;
}

std::vector<EdECalculator::EdEDepthEnvelope> EdECalculator::Calculate(
    const EdEDetectorDefinition &detector, const std::vector<double> &excitationEnergies)
{
    std::vector<EdEDepthEnvelope> result;
    result.reserve(excitationEnergies.size());
    for (const double excitation : excitationEnergies)
        result.emplace_back(Calculate(detector, excitation));
    return result;
}

std::vector<double> EdECalculator::MakeExcitationGrid(double minimum, double maximum, double step)
{
    if (minimum < 0 || maximum < minimum || step <= 0)
        throw EdEConfigurationError("Invalid E-dE excitation grid");

    std::vector<double> result;
    const std::size_t   count =
        static_cast<std::size_t>(std::floor((maximum - minimum) / step + 1.e-9));
    result.reserve(count + 1);
    for (std::size_t index = 0; index <= count; ++index)
        result.emplace_back(minimum + static_cast<double>(index) * step);

    if (result.empty() || result.back() < maximum - step * 1.e-9)
        result.emplace_back(maximum);
    return result;
}
