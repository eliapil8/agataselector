#pragma once

#include <algorithm>
#include <array>
#include <map>

#include "DetectorConf.h"
#include "OscarLut.h"
#include "ReactionFragment.h"

class OscarConf : public DetectorConf
{
  public:
    OscarConf() = delete;

  protected:
    explicit OscarConf(const std::string &mark);

  public:
    bool                             useElayerOnly{false};
    bool                             dE_E_Histos{false};
    bool                             global_dE_E_Cuts{false};
    bool                             rejectOutsideDEEnergy{false};
    bool                             rejectOutsideEnergyTheta{false};
    bool                             EkFromBetaAvg{false};
    double                           eShift{0};
    double                           eGain{1};
    double                           oscarbinWidth{.1};
    double                           oscarEmax{200 * UNITS::MeV};
    double                           oscarExmax{90 * UNITS::MeV};
    double                           oscarExbinWidth{0.1};
    double                           deadlayerThickness{0 * UNITS::ug / UNITS::cm2};
    std::string                      deadlayerMaterial{"27-13"};
    bool                             edeKinematicLines{false};
    std::string                      edeReactionFile{"EdECalculatorOscar.conf"};
    std::string                      edeActiveMaterial{"28-14"};
    std::string                      energyThetaCutPath;
    std::vector<TCutG *>             energyThetaCuts;
    std::vector<std::pair<double, double>> excitationEnergyGates;
    ReactionFragment::Ion            ion;
    OscarLut                         lut;
    std::vector<std::pair<int, int>> brokenChannels{{-1, -1}};

    struct
    {
        // Oscar DIMENSION DE
        const double LayerDE_Y      = 52.0 * UNITS::mm;
        const double StripDE_X      = 3.125 * UNITS::mm;
        const double StripDE_Y      = 52.0 * UNITS::mm;
        const double InterStripDE_X = 0.125 * UNITS::mm;
        const double pitchDE_X      = StripDE_X + InterStripDE_X;
        const double LayerDE_X      = StripDE_X * 16 + InterStripDE_X * 15;
        // const double ThicknessDE = 20.0*um;

        // Oscar DIMENSION E
        const double PadE_X       = 10.0 * UNITS::mm;
        const double PadE_Y       = 10.0 * UNITS::mm;
        const double InterPadE_X  = 1.4 * UNITS::mm;
        const double InterPadE_Ys = 1.4 * UNITS::mm; // short
        const double InterPadE_Yl = 3.2 * UNITS::mm; // long
        const double pitchE_X     = 2 * InterPadE_X + PadE_X;
        const double pitchE_Y     = InterPadE_Ys + InterPadE_Yl + PadE_Y;
        const double LayerE_X     = pitchE_X * 4;
        const double LayerE_Y     = pitchE_Y * 4;
        // const double ThicknessE = 1.0;
        // const double LayerDist = 0.2;
    } GEOMETRY;

    inline const OscarLut::Layer *getOscarLayer(const unsigned short b,
                                                const unsigned short ch) const
    {
        const auto bch = OscarLut::encodeBoardChannel(b, ch);
        auto       it  = lut.fMap_bch_layer.find(bch);
        if (it == lut.fMap_bch_layer.end())
            return nullptr;
        else
            return &(it->second);
    }

    inline const OscarLut::Telescope *getOscarTelescope(const unsigned short stripId,
                                                        const unsigned short padId) const
    {
        const auto key = OscarLut::encodeStripPad(stripId, padId);
        auto       it  = lut.fMap_strippad_telescope.find(key);
        if (it == lut.fMap_strippad_telescope.end())
            return nullptr;
        else
            return &(it->second);
    }

    inline std::vector<const OscarLut::Telescope *> getOscarTelescopes() const
    {
        std::vector<const OscarLut::Telescope *> telescopes;
        telescopes.reserve(lut.fMap_strippad_telescope.size());
        for (const auto &it : lut.fMap_strippad_telescope)
        {
            telescopes.emplace_back(&it.second);
        }
        std::sort(telescopes.begin(), telescopes.end(), [](const auto *lhs, const auto *rhs) {
            return lhs->telescopeID < rhs->telescopeID;
        });
        return telescopes;
    }

    inline std::vector<const OscarLut::Layer *> getOscarStrips() const
    {
        std::vector<const OscarLut::Layer *> strips;
        strips.reserve(lut.fMap_bch_strip.size());
        for (const auto &it : lut.fMap_bch_strip)
        {
            strips.emplace_back(&it.second);
        }
        std::sort(strips.begin(), strips.end(), [](const auto *lhs, const auto *rhs) {
            return lhs->selector_id < rhs->selector_id;
        });
        return strips;
    }

    inline std::vector<const OscarLut::Layer *> getOscarPads() const
    {
        std::vector<const OscarLut::Layer *> pads;
        pads.reserve(lut.fMap_bch_pad.size());
        for (const auto &it : lut.fMap_bch_pad)
        {
            pads.emplace_back(&it.second);
        }
        std::sort(pads.begin(), pads.end(), [](const auto *lhs, const auto *rhs) {
            return lhs->selector_id < rhs->selector_id;
        });
        return pads;
    }

    inline double getEnergy(const OscarLut::Layer *oscar_layer, double raw_energy) const
    {
        return eGain * (oscar_layer->getEnergy(raw_energy)) + eShift;
    }

    inline int getTSOffset(const OscarLut::Layer *oscar_layer) const
    {
        return oscar_layer->TSOffset;
    }

    inline TVector3 getNormalDir() const { return lut.getNormalDir(); }

    inline ReactionFragment::Ion idParticleFrom_dE_E(const double &e, const double &de) const
    {
        return lut.idParticleFrom_dE_E(e, de);
    }
    inline ReactionFragment::Ion idParticleFrom_dE_E(const double      &e,
                                                     const double      &de,
                                                     const unsigned int telescopeId) const
    {
        return lut.idParticleFrom_dE_E(e, de, telescopeId);
    }

    /**
     * Test whether an excitation energy passes the configured OSCAR gate.
     *
     * An empty gate list disables excitation-energy filtering. If EX_GATE is
     * repeated, an energy passing any configured interval is accepted.
     *
     * @param excitationEnergy Reconstructed excitation energy in internal units.
     * @return True when filtering is disabled or the energy is inside a gate.
     */
    bool passesExcitationEnergyGate(double excitationEnergy) const;

    /**
     * Test whether a reconstructed particle lies inside any E-theta banana.
     *
     * Cut coordinates follow the OSCAR matrix axes: laboratory theta in
     * degrees and reaction-point particle energy in MeV.
     *
     * @param thetaLab Laboratory particle angle in internal angular units.
     * @param particleEnergy Reaction-point particle kinetic energy in internal units.
     * @return True when the point lies inside at least one loaded cut.
     */
    bool passesEnergyThetaCut(double thetaLab, double particleEnergy) const;

    /** @return True when at least one E-dE particle-identification cut is loaded. */
    bool hasDEEnergyCuts() const { return lut.hasDEEnergyCuts(); }

    inline std::string getName(const unsigned short brd, const unsigned short ch) const
    {
        return lut.getName(brd, ch);
    }
};

class OscarUpConf : public OscarConf
{
  public:
    OscarUpConf() : OscarConf("OSCARUP_CONF")
    {
        energyThetaCutPath = "/CUT/OSCARUP/ETheta/";
    };
};

class OscarDownConf : public OscarConf
{
  public:
    OscarDownConf() : OscarConf("OSCARDOWN_CONF")
    {
        energyThetaCutPath = "/CUT/OSCARDOWN/ETheta/";
    };
};
