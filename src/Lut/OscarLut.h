#pragma once

#include <cassert>
#include <exception>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <TCutG.h>
#include <TVector3.h>

#include "Lut.h"
#include "ReactionFragment.h"
#include "Settings.h"

class ConfManager;

class OscarLut : public Lut
{
    friend class OscarConf;
    friend class ConfManager;

  public:
    OscarLut();
    void ReadLut(const std::string &) override;
    void PrintLut(std::ostream &outstream = std::cout) override;
    void Load_dE_E_Cuts(const std::vector<TCutG *> &);
    void Load_dE_E_Cuts_TelescopeID(const std::vector<TCutG *> &);

    /** @return True when global or per-telescope E-dE cuts have been loaded. */
    bool hasDEEnergyCuts() const
    {
        return !fpId_EdE.empty() || !fMap_telescopeID_pId_EdE.empty();
    }

    enum OscarLayer
    {
        DELTA_E,
        E_RES
    };

  public:
    const Settings &settings;
    struct Layer
    {
        OscarLayer          type;
        unsigned short      board;
        unsigned short      channel;
        unsigned int        selector_id; // calculated from board and channel
        unsigned short      layerID;     // can be strip(DE) or pad(Eres)
        std::string         name;
        double              thr_low;
        double              thr_high;
        int                 TSOffset;
        std::vector<double> cal;
        double              getEnergy(const double raw_energy) const
        {
            double cal_en{0};
            for (unsigned int i{0}; i < cal.size(); ++i)
            {
                cal_en += cal[i] * pow(raw_energy, i);
            }
            return cal_en;
        };
        // ULong64_t getTimestamp(const ULong64_t ts) { return ts + TSOffset; }
    };

    struct Telescope
    {
        unsigned int telescopeID; // can be strip(DE) or pad(Eres)
        Layer       *layer_dE;
        Layer       *layer_Eres;
        std::string  name;
        double       pos_x;  // in mm
        double       pos_y;  // in mm
        double       pos_z;  // in mm
        double       norm_x; // normalized
        double       norm_y; // normalized
        double       norm_z; // normalized
        double       thick_dead_dE;
        double       thick_active_dE;
        double       thick_dead_Eres;
        double       thick_active_Eres;
        double       timeOffset; // in ns
    };

  private:
    std::vector<std::pair<ReactionFragment::Ion, TCutG *>> fpId_EdE;
    // std::unordered_map<unsigned int,std::unordered_map<unsigned int,
    // std::vector<std::pair<ReactionFragment::Ion, TCutG*>>>>
    //     pId_dE_E_pad_strip;

    std::unordered_map<unsigned int, std::vector<std::pair<ReactionFragment::Ion, TCutG *>>>
        fMap_telescopeID_pId_EdE;

    // these integers are combination of 2 shorts
    std::unordered_map<int, Layer> fMap_bch_strip;
    std::unordered_map<int, Layer> fMap_bch_pad;
    std::unordered_map<int, Layer> fMap_bch_layer;
    // this key integer is telescope ID
    std::unordered_map<int, Telescope> fMap_strippad_telescope;

    // std::unordered_map<unsigned int, std::unordered_map<unsigned int,
    // Properties>>
    //               prop;

    // encode board channel into 32bit number so that map can use it directly
    // as hash
  public:
    static int encodeBoardChannel(const unsigned short b, const unsigned short ch)
    {
        return (b << 16) | ch;
    }

    static void decodeBoardChannel(int key, unsigned short &b, unsigned short &ch)
    {
        b  = static_cast<short>(key >> 16);
        ch = static_cast<short>(key);
    }

    static int encodeStripPad(const unsigned short strip, const unsigned short pad)
    {
        return (strip << 16) | pad;
    }

    static void decodeStripPad(int key, unsigned short &strip, unsigned short &pad)
    {
        strip = static_cast<short>(key >> 16);
        pad   = static_cast<short>(key);
    }

    std::string getName(const unsigned short &brd, const unsigned short &ch) const
    {
        auto key = this->encodeBoardChannel(brd, ch);
        auto it  = fMap_bch_layer.find(key);
        if (it == fMap_bch_layer.end())
        {
            return "NA";
        }
        return it->second.name;
    }

    TVector3 getNormalDir() const
    {
        return TVector3(fMap_strippad_telescope.begin()->second.norm_x,
                        fMap_strippad_telescope.begin()->second.norm_y,
                        fMap_strippad_telescope.begin()->second.norm_z);
    }

    inline ReactionFragment::Ion idParticleFrom_dE_E(const double &e, const double &de) const
    {
        ReactionFragment::Ion ion(0, 0);
        bool                  alreadyFound{false};
        for (const auto &it : fpId_EdE)
        {
            // std::cout << "oscar e " << e << " oscar de " << de << std::endl;
            // std::cout << " in cut? " << it.first->Z << " " << it.first->A <<
            // std::endl;
            if (it.second->IsInside(e, de))
            {
                if (alreadyFound)
                {
                    throw std::runtime_error(std::string("Overlapping OSCAR dE vs E gates\n"));
                }

                alreadyFound = true;
                ion          = it.first;
            }
        }
        return ion;
    };

    inline ReactionFragment::Ion idParticleFrom_dE_E(const double      &e,
                                                     const double      &de,
                                                     const unsigned int telescopeId) const
    {
        ReactionFragment::Ion ion(0, 0);

        // check if cuts are present
        auto it_pId = fMap_telescopeID_pId_EdE.find(telescopeId);
        if (it_pId == fMap_telescopeID_pId_EdE.end())
        {
            return ion;
        }
        bool alreadyFound{false};

        for (const auto &it : it_pId->second)
        {
            if (it.second->IsInside(e, de))
            {
                if (alreadyFound)
                {
                    throw std::runtime_error("Overlapping OSCAR Delta E vs E gates: ion " +
                                             std::to_string(ion.A) + "-" + std::to_string(ion.Z) +
                                             " and ion " + std::to_string(it.first.A) + "-" +
                                             std::to_string(it.first.Z) + " for telescope " +
                                             std::to_string(telescopeId) + "\n");
                }

                alreadyFound = true;
                ion          = it.first;
            }
        }
        return ion;
    };
};
