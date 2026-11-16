#pragma once

#include "EdECalculator.h"

#include <TGraphAsymmErrors.h>

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class OscarConf;
class ReactionConf;
class ReplayConf;
class TargetConf;
class TDirectory;
class TFile;
class TH2F;

/**
 * OSCAR adapter for the detector-independent E-dE calculator.
 *
 * The adapter converts selector target and OSCAR LUT configuration into the
 * generic material and detector descriptions, owns one EdECalculator per
 * reaction, and creates the ROOT objects used as E-dE overlays.
 */
class EdECalculatorOscar
{
  public:
    /** Material layer in which an OSCAR overlay reaction occurs. */
    enum class EdEReactionLayer
    {
        TARGET,
        BACKING
    };

    /** Inclusive regular excitation-energy scan, expressed in MeV. */
    struct EdEExcitationGrid
    {
        double minimum{0};
        double maximum{0};
        double step{0};
    };

    /** Reaction construction method used by the OSCAR adapter. */
    enum class EdEReactionMechanism
    {
        TRANSFER,
        BINARY,
        ELASTIC
    };

    /** Kinematic solution or solutions requested by one OSCAR reaction block. */
    enum class EdEBranchSelection
    {
        HIGH_ENERGY,
        LOW_ENERGY,
        BOTH
    };

    /** One reaction and its requested visualization modes. */
    struct EdEOscarReaction
    {
        std::string                    name;
        EdEReactionLayer               layer{EdEReactionLayer::TARGET};
        EdECalculator::Ion             target{0, 0};
        EdEReactionMechanism           mechanism{EdEReactionMechanism::TRANSFER};
        unsigned int                   transferredA{0};
        unsigned int                   transferredZ{0};
        EdECalculator::Ion             detected{0, 0};
        EdEBranchSelection              branch{EdEBranchSelection::HIGH_ENERGY};
        bool                           hasGrid{false};
        EdEExcitationGrid              grid;
        std::vector<double>            knownLevels;
        /** Symmetric visual E half-width in MeV, combined with depth uncertainty. */
        double energyResolution{0};
        /** Symmetric visual dE half-width in MeV, combined with depth uncertainty. */
        double deltaEResolution{0};
        int                            lineColor{0};
        int                            lineStyle{1};
        int                            lineWidth{2};
        int                            markerStyle{20};
        double                         markerSize{0.8};
    };

    /** Kind of excitation sequence represented by a generated graph. */
    enum class EdEGraphMode
    {
        SMOOTH_GRID,
        KNOWN_LEVELS,
        ELASTIC_REGION
    };

    /** Horizontal coordinate represented by a generated E-dE graph. */
    enum class EdEGraphAxes
    {
        /** Detected fragment energy at the reaction point versus dE. */
        REACTION_ENERGY_DELTA_E,
        /** Energy deposited in OSCAR's residual-E layer versus dE. */
        RESIDUAL_E_DELTA_E
    };

    /** Metadata and ownership for one generated ROOT graph. */
    struct EdEGraph
    {
        std::string                        reactionName;
        unsigned int                       telescopeID{0};
        EdEGraphMode                       mode{EdEGraphMode::SMOOTH_GRID};
        EdEGraphAxes                       axes{EdEGraphAxes::REACTION_ENERGY_DELTA_E};
        std::unique_ptr<TGraphAsymmErrors> graph;
    };

    /**
     * Construct an OSCAR adapter using the selector's live configuration.
     *
     * @param reactionConf Beam ion and entrance energy configuration.
     * @param targetConf Target/backing geometry and stopping-table type.
     * @param replayConf Configuration directory containing nuclear and loss data.
     * @param oscarConf OSCAR LUT, geometry corrections, and detector materials.
     */
    EdECalculatorOscar(const ReactionConf &reactionConf,
                       const TargetConf   &targetConf,
                       const ReplayConf   &replayConf,
                       const OscarConf    &oscarConf);

    /** Destroy all reaction calculators and generated graphs. */
    ~EdECalculatorOscar();

    /** The adapter owns non-copyable calculators and ROOT graphs. */
    EdECalculatorOscar(const EdECalculatorOscar &) = delete;

    /** The adapter owns non-copyable calculators and ROOT graphs. */
    EdECalculatorOscar &operator=(const EdECalculatorOscar &) = delete;

    /**
     * Replace the reaction list with definitions parsed from a text file.
     *
     * @param path Full path of the OSCAR E-dE reaction-definition file.
     * @throws EdECalculator::EdEConfigurationError for malformed input.
     */
    void LoadReactionFile(const std::string &path);

    /**
     * Add one reaction programmatically.
     *
     * This supports reactions not present in the selector's main reaction
     * list. Each added definition receives its own EdECalculator instance.
     *
     * @param reaction Reaction, excitation sequences, and graph style.
     * @throws EdECalculator::EdEConfigurationError for an invalid definition.
     */
    void AddReaction(EdEOscarReaction reaction);

    /**
     * Validate all required stopping tables and generate every OSCAR graph.
     *
     * Existing calculators and graphs are replaced. Invalid physical points
     * are omitted; configuration and stopping-table errors abort generation.
     *
     * @throws EdECalculator::EdEConfigurationError if setup is incomplete.
     */
    void BuildGraphs();

    /**
     * Draw and save one overlay canvas per available telescope and axis family.
     *
     * Each canvas contains the filled matrix, all configured reaction graphs,
     * and a reaction legend. Graphs are serialized only as canvas primitives,
     * not as separate ROOT keys.
     *
     * @param reactionEnergyParent Directory containing corrected total-energy-dE matrices.
     * @param reactionEnergyMatrices Corrected matrices keyed by telescope ID.
     * @param residualEnergyParent Directory containing true residual-E-dE matrices.
     * @param residualEnergyMatrices Residual-E matrices keyed by telescope ID.
     * @throws EdECalculator::EdEConfigurationError if graphs were not built.
     */
    void WriteCanvases(
        TDirectory                                     &reactionEnergyParent,
        const std::unordered_map<unsigned int, TH2F *> &reactionEnergyMatrices,
        TDirectory                                     &residualEnergyParent,
        const std::unordered_map<unsigned int, TH2F *> &residualEnergyMatrices) const;

    /**
     * Find OSCAR matrices in a completed ROOT file and add overlay canvases.
     *
     * This overload is intended for the single-threaded post-merge phase. The
     * file must be open for update and BuildGraphs() must already have run.
     *
     * @param file Completed or hadded ROOT output file.
     * @param oscarDirectory Top-level detector directory, e.g. OscarUp.
     * @throws EdECalculator::EdEConfigurationError if required directories are absent.
     */
    void WriteCanvases(TFile &file, const std::string &oscarDirectory) const;

    /** @return Generated graphs and their OSCAR/reaction metadata. */
    const std::vector<EdEGraph> &GetGraphs() const { return graphs; }

    /** @return Reactions currently configured for graph generation. */
    const std::vector<EdEOscarReaction> &GetReactions() const { return reactions; }

  private:
    const ReactionConf &reactionConf;
    const TargetConf   &targetConf;
    const ReplayConf   &replayConf;
    const OscarConf    &oscarConf;

    std::vector<EdEOscarReaction>                     reactions;
    std::vector<EdECalculator::EdEMaterialLayer>      materialStack;
    std::vector<EdECalculator::EdEDetectorDefinition> detectors;
    std::vector<std::unique_ptr<EdECalculator>>       calculators;
    std::vector<EdEGraph>                             graphs;
    std::size_t                                       targetLayerIndex{0};
    std::size_t                                       backingLayerIndex{0};
    bool                                              hasBacking{false};
    bool                                              graphsBuilt{false};

    /** Build the beam-ordered target/backing material stack. */
    void BuildMaterialStack();

    /** Convert all active OSCAR LUT telescopes into generic detector definitions. */
    void BuildDetectorDefinitions();

    /** Validate fields that are specific to one OSCAR reaction definition. */
    void ValidateReaction(const EdEOscarReaction &reaction) const;

    /** Resolve a configured reaction layer to its beam-ordered stack index. */
    std::size_t ResolveReactionLayer(EdEReactionLayer layer) const;

    /**
     * Create and append one open asymmetric-error graph.
     *
     * @param reaction OSCAR reaction and visualization settings.
     * @param calculator Single-branch physics calculator used for the points.
     * @param detector Telescope geometry and detector-layer definition.
     * @param excitationEnergies Excitation energies in internal energy units.
     * @param mode Excitation sequence or elastic-region visualization mode.
     * @param axes Horizontal coordinate to place in the graph.
     * @param branchSuffix Optional ROOT-name suffix distinguishing two branches.
     * @param legendSuffix Optional title and legend suffix distinguishing two branches.
     */
    void BuildGraph(const EdEOscarReaction                     &reaction,
                    EdECalculator                              &calculator,
                    const EdECalculator::EdEDetectorDefinition &detector,
                    const std::vector<double>                  &excitationEnergies,
                    EdEGraphMode                                mode,
                    EdEGraphAxes                                axes,
                    const std::string                          &branchSuffix = "",
                    const std::string                          &legendSuffix = "");

    /** Draw and save one coordinate family's telescope canvases. */
    void WriteCanvasFamily(TDirectory                                     &parent,
                           const std::unordered_map<unsigned int, TH2F *> &matrices,
                           EdEGraphAxes                                    axes) const;

    /** Return an explicit color or the cyclic KRATTA reaction color. */
    static int ReactionColor(const EdEOscarReaction &reaction, std::size_t index);

    /** Map the selector energy-loss table code to EnergyLoss's source name. */
    std::string EnergyLossTableType() const;

    /** Convert arbitrary reaction names into ROOT-safe object names. */
    static std::string SafeName(const std::string &name);
};
