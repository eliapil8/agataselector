#pragma once

#include "EnergyLoss.h"
#include "ReactionFragment.h"
#include "ReactionReconstruction.h"

#include <TVector3.h>

#include <cstddef>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

/**
 * Detector-independent calculation of theoretical E-dE points.
 *
 * EdECalculator deliberately has no dependency on selector detector
 * configuration, LUT classes, histograms, or ROOT graphics.  Detector
 * adapters provide an ordered material stack, a reaction definition, and a
 * generic telescope description.
 */
class EdECalculator
{
  public:
    using Ion = ReactionFragment::Ion;

    enum class ThicknessType
    {
        LENGTH,
        AREAL_DENSITY
    };

    enum class DetectorLayerRole
    {
        PASSIVE,
        DELTA_E,
        RESIDUAL_E
    };

    enum class KinematicBranch
    {
        HIGH_ENERGY,
        LOW_ENERGY
    };

    enum class StopLocation
    {
        NONE,
        TARGET_STACK,
        PASSIVE_DETECTOR_LAYER,
        DELTA_E_LAYER,
        RESIDUAL_E_LAYER
    };

    struct EdESettings
    {
        std::string nuclearDataFile;
        std::string energyLossDirectory;
        std::string energyLossTableType{"SRIM"};
        int         energyLossSlices{100};
    };

    /** A layer in the target/backing stack, ordered along the beam. */
    struct EdEMaterialLayer
    {
        std::string name;
        double      arealThickness{0};
        TVector3    normal{0, 0, 1};
    };

    /** A passive or active layer of an E-dE telescope. */
    struct EdEDetectorLayer
    {
        std::string       name;
        std::string       material;
        double            thickness{0};
        ThicknessType     thicknessType{ThicknessType::LENGTH};
        DetectorLayerRole role{DetectorLayerRole::PASSIVE};
    };

    struct EdEDetectorDefinition
    {
        unsigned int                  id{0};
        std::string                   name;
        TVector3                      direction{0, 0, 1};
        TVector3                      normal{0, 0, 1};
        std::vector<EdEDetectorLayer> layers;
    };

    /**
     * Binary reaction p1 + p2 -> residual + detected.
     *
     * reactionLayerIndex selects the material layer in which the reaction
     * takes place. entranceBeamEnergy is the energy before layer zero.
     */
    struct EdEReactionDefinition
    {
        std::string     name;
        Ion             beam{0, 0};
        Ion             target{0, 0};
        Ion             detected{0, 0};
        Ion             residual{0, 0};
        double          entranceBeamEnergy{0};
        TVector3        beamDirection{0, 0, 1};
        std::size_t     reactionLayerIndex{0};
        double          nominalDepth{0.5};
        KinematicBranch branch{KinematicBranch::HIGH_ENERGY};
    };

    struct EdEPoint
    {
        double       excitationEnergy{0};
        double       reactionDepth{0};
        double       beamEnergyAtReaction{0};
        double       energyAtReaction{0};
        double       energyAtDetector{0};
        double       deltaEDeposit{0};
        double       residualEDeposit{0};
        double       totalActiveDeposit{0};
        double       punchThroughEnergy{0};
        StopLocation stopLocation{StopLocation::NONE};
        bool         valid{false};
        std::string  diagnostic;
    };

    struct EdEAsymmetricError
    {
        double lower{0};
        double upper{0};
    };

    struct EdEDepthEnvelope
    {
        EdEPoint           front;
        EdEPoint           center;
        EdEPoint           back;
        EdEAsymmetricError beamEnergyAtReactionError;
        EdEAsymmetricError energyAtReactionError;
        EdEAsymmetricError energyAtDetectorError;
        EdEAsymmetricError deltaEError;
        EdEAsymmetricError residualEError;
        EdEAsymmetricError totalActiveDepositError;
        bool               valid{false};
        std::string        diagnostic;
    };

    class EdEConfigurationError : public std::runtime_error
    {
      public:
        /**
         * Construct an exception describing an invalid calculator setup.
         *
         * @param message Human-readable diagnostic.
         */
        explicit EdEConfigurationError(const std::string &message) : std::runtime_error(message) {}
    };

    /**
     * Construct a calculator for one reaction using the supplied data sources.
     *
     * No files are opened until Validate() or Calculate() is called.
     *
     * @param settings Nuclear-data and stopping-table settings.
     * @param reaction The single reaction calculated by this instance.
     * @throws EdEConfigurationError if @p reaction is structurally invalid.
     */
    EdECalculator(EdESettings settings, EdEReactionDefinition reaction);

    /** Destroy the calculator and its cached stopping tables and reconstruction. */
    ~EdECalculator();

    /** The calculator owns non-copyable stopping-table caches. */
    EdECalculator(const EdECalculator &) = delete;

    /** The calculator owns non-copyable stopping-table caches. */
    EdECalculator &operator=(const EdECalculator &) = delete;

    /** Move-construct a calculator, including its caches. */
    EdECalculator(EdECalculator &&) = default;

    /** Move-assign a calculator, including its caches. */
    EdECalculator &operator=(EdECalculator &&) = default;

    /**
     * Set the ordered target/backing stack traversed along the beam direction.
     *
     * Layer zero is encountered first by the incoming beam. A reaction's
     * reactionLayerIndex selects the layer in which it occurs.
     *
     * @param layers Material layers with areal thicknesses in internal units.
     */
    void SetMaterialStack(std::vector<EdEMaterialLayer> layers);

    /**
     * Build a binary reaction and derive the complementary residual fragment.
     *
     * @param name Descriptive reaction name used in diagnostics.
     * @param beam Incoming projectile ion.
     * @param target Target ion at rest.
     * @param detected Outgoing ion transported to the detector.
     * @param entranceBeamEnergy Beam kinetic energy before material layer zero,
     *        in internal energy units.
     * @param reactionLayerIndex Index of the layer in which the reaction occurs.
     * @param nominalDepth Nominal reaction depth as a fraction in [0,1].
     * @param branch Kinematic solution used when two laboratory solutions exist.
     * @return A validated reaction definition.
     * @throws EdEConfigurationError if an ion is invalid, conservation fails, or
     *         another reaction field is invalid.
     */
    static EdEReactionDefinition MakeBinaryReaction(
        const std::string &name,
        const Ion         &beam,
        const Ion         &target,
        const Ion         &detected,
        double             entranceBeamEnergy,
        std::size_t        reactionLayerIndex,
        double             nominalDepth = 0.5,
        KinematicBranch    branch       = KinematicBranch::HIGH_ENERGY);

    /**
     * Build a transfer reaction in which (transferredA, transferredZ) moves
     * from the projectile to the target-like product.
     *
     * @param name Descriptive reaction name used in diagnostics.
     * @param beam Incoming projectile ion.
     * @param target Target ion at rest.
     * @param transferredA Mass number transferred from beam to target.
     * @param transferredZ Charge transferred from beam to target.
     * @param entranceBeamEnergy Beam kinetic energy before material layer zero,
     *        in internal energy units.
     * @param reactionLayerIndex Index of the layer in which the reaction occurs.
     * @param nominalDepth Nominal reaction depth as a fraction in [0,1].
     * @param branch Kinematic solution used when two laboratory solutions exist.
     * @return A validated reaction definition whose detected fragment is the
     *         projectile-like product.
     * @throws EdEConfigurationError if the transfer is invalid or the resulting
     *         binary reaction is inconsistent.
     */
    static EdEReactionDefinition MakeTransferReaction(
        const std::string &name,
        const Ion         &beam,
        const Ion         &target,
        unsigned int       transferredA,
        unsigned int       transferredZ,
        double             entranceBeamEnergy,
        std::size_t        reactionLayerIndex,
        double             nominalDepth = 0.5,
        KinematicBranch    branch       = KinematicBranch::HIGH_ENERGY);

    /** @return The target/backing stack in beam traversal order. */
    const std::vector<EdEMaterialLayer> &GetMaterialStack() const { return materialStack; }

    /** @return The reaction owned by this calculator. */
    const EdEReactionDefinition &GetReaction() const { return reaction; }

    /**
     * Validate the reaction and all stopping tables needed by the supplied
     * detectors. All detected problems are reported together.
     *
     * @param detectors Generic detector descriptions that will be calculated.
     * @throws EdEConfigurationError containing all discovered configuration,
     *         nuclear-data, and stopping-table problems.
     */
    void Validate(const std::vector<EdEDetectorDefinition> &detectors);

    /**
     * Calculate one E-dE point and its reaction-depth uncertainty envelope.
     *
     * The central value uses EdEReactionDefinition::nominalDepth; the two error
     * limits use the front and back faces of the reaction layer.
     *
     * @param detector Detector direction, normal, and ordered layer stack.
     * @param excitationEnergy Excitation assigned to the target-like residual,
     *        in internal energy units.
     * @return Front, central, and back results with asymmetric errors. The
     *         returned envelope is invalid when no physical result exists or
     *         the fragment fails to reach the detector at one of the depths.
     * @throws EdEConfigurationError for invalid setup or unavailable stopping data.
     */
    EdEDepthEnvelope Calculate(const EdEDetectorDefinition &detector, double excitationEnergy);

    /**
     * Calculate depth envelopes for an arbitrary excitation-energy sequence.
     *
     * This overload supports both regular scans and explicit known-level lists.
     * Results preserve the order of @p excitationEnergies.
     *
     * @param detector Detector direction, normal, and ordered layer stack.
     * @param excitationEnergies Excitations in internal energy units.
     * @return One depth envelope per requested excitation energy.
     * @throws EdEConfigurationError for invalid setup or unavailable stopping data.
     */
    std::vector<EdEDepthEnvelope> Calculate(const EdEDetectorDefinition &detector,
                                            const std::vector<double>   &excitationEnergies);

    /**
     * Construct an inclusive, regularly spaced excitation-energy grid.
     *
     * If @p step does not divide the range exactly, @p maximum is appended as
     * the final point.
     *
     * @param minimum First excitation energy in internal units.
     * @param maximum Last excitation energy in internal units.
     * @param step Positive spacing in internal units.
     * @return Inclusive excitation-energy sequence.
     * @throws EdEConfigurationError if the range or step is invalid.
     */
    static std::vector<double> MakeExcitationGrid(double minimum, double maximum, double step);

    /**
     * Resolve the expected stopping-table filename for an ion/material pair.
     *
     * @param material Material identifier used in Target-&lt;material&gt; filenames.
     * @param ion Projectile ion represented by the stopping table.
     * @return Full table path according to the configured directory.
     */
    std::string EnergyLossTablePath(const std::string &material, const Ion &ion) const;

  private:
    struct EnergyLossKey
    {
        std::string material;
        Ion         ion{0, 0};

        /** @return Strict ordering used by the stopping-table cache. */
        bool operator<(const EnergyLossKey &other) const;
    };

    EdESettings                                               settings;
    EdEReactionDefinition                                     reaction;
    std::vector<EdEMaterialLayer>                             materialStack;
    std::map<EnergyLossKey, std::unique_ptr<EnergyLoss>>      energyLossTables;
    std::unique_ptr<ReactionReconstruction2body<long double>> reactionPrototype;

    /** Validate fields and conservation laws of a reaction definition. */
    static void ValidateReactionStructure(const EdEReactionDefinition &reaction);

    /** Validate geometry, materials, and active-layer roles of a detector. */
    static void ValidateDetectorStructure(const EdEDetectorDefinition &detector);

    /** Validate the material-stack fields independently of any reaction. */
    void ValidateMaterialStack() const;

    /** Load or retrieve the cached stopping table for an ion/material pair. */
    EnergyLoss &GetEnergyLoss(const std::string &material, const Ion &ion);

    /** Build or retrieve the cached two-body reconstruction prototype. */
    ReactionReconstruction2body<long double> &GetReactionPrototype();

    /** Calculate a single point at an explicit fractional reaction depth. */
    EdEPoint CalculateAtDepth(const EdEDetectorDefinition &detector,
                              double                       excitationEnergy,
                              double                       reactionDepth);

    /** Calculate incoming-beam energy at an explicit reaction depth. */
    double BeamEnergyAtDepth(double reactionDepth);

    /** Transport the detected fragment from its reaction point to the detector. */
    double TransportOutOfTarget(const EdEDetectorDefinition &detector,
                                double                       energy,
                                double                       reactionDepth,
                                StopLocation                &stopLocation);

    /** Transport a point through passive, dE, and residual-E detector layers. */
    void TransportThroughDetector(const EdEDetectorDefinition &detector, EdEPoint &point);

    /**
     * Slow an ion through one material thickness using a cached table.
     *
     * @return Remaining kinetic energy in internal units.
     */
    double Slow(const std::string &material,
                const Ion         &ion,
                double             energy,
                double             thickness,
                ThicknessType      thicknessType,
                double             angle);

    /** Convert front/back values into lower and upper errors about the center. */
    static EdEAsymmetricError MakeError(double center, double front, double back);

    /** @return An ion formatted as the stopping-table A-Z identifier. */
    static std::string IonName(const Ion &ion);
};
