#pragma once

#include <array>

#include "DetectorConf.h"
#include "EuclidesLut.h"
#include "ReactionFragment.h"

class EuclidesConf : public DetectorConf {
  public:
    EuclidesConf();

    double        eShift{0};
    double        eGain{1};
    EuclidesLut   lut;
    // std::array<double, 5> timeExtr{};
    inline double getEnergy(const double& val, const unsigned int& brd,
                            const unsigned int& ch) const {
        return eGain * (lut.getEnergy(val, brd, ch)) + eShift;
    }
    inline double getTime(const double& val, const unsigned int& brd,
                          const unsigned int& ch) const {
        return lut.getTime(val, brd, ch);
    }
    inline TVector3
    getPos(const unsigned int& brd, const unsigned int& ch) const {
        return lut.getPos(brd, ch);
    }
    inline std::pair<double, double>
    getThr(const unsigned int& brd, const unsigned int& ch) const {
        return lut.getThr(brd, ch);
    }
    inline std::string
    getName(const unsigned int& brd, const unsigned int& ch) const {
        return lut.getName(brd, ch);
    }
    inline unsigned int
    getMapName(const unsigned int& brd, const unsigned int& ch) const {
        return lut.getMap(brd, ch);
    }
    inline bool isE(const unsigned int& brd, const unsigned int& ch) const {
        return lut.isE(brd, ch);
    }
    inline bool isdE(const unsigned int& brd, const unsigned int& ch) const {
        return lut.isdE(brd, ch);
    }
    inline unsigned int
    getRing(const unsigned int& brd, const unsigned int& ch) const {
        return lut.getRing(brd, ch);
    }
    inline unsigned int
    getDet(const unsigned int& brd, const unsigned int& ch) const {
        return lut.getDet(brd, ch);
    }
    inline unsigned int
    getSeg(const unsigned int& brd, const unsigned int& ch) const {
        return lut.getSeg(brd, ch);
    }
    inline ReactionFragment::Ion
    idParticle(const double& e, const double& de, const unsigned int& brd,
               const unsigned int& ch) const {
        return lut.idParticle(e, de, brd, ch);
    }
    std::unordered_map<unsigned int, std::string> getListOfNames_dE() const {
        std::unordered_map<unsigned int, std::string> names;
        for(const auto& it : lut.prop) {
            if(it.second.isdE) names[it.second.map_detID] = it.second.name;
        }
        return names;
    }
    std::unordered_map<ReactionFragment::Ion, bool,
                       ReactionFragment::Ion::hash_fn>
    getListOfParticles() const {
        std::unordered_map<ReactionFragment::Ion, bool,
                           ReactionFragment::Ion::hash_fn>
            particles;
        for(const auto& it : lut.prop) {
            for(const auto& p : it.second.pId) { particles[p.first] = true; }
        }
        return particles;
    }
};
