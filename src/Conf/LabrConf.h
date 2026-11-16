#pragma once

#include <array>

#include "GammadetectorConf.h"
#include "LabrLut.h"
#include "ReactionFragment.h"

class LabrConf : public GammadetectorConf {
  public:
    LabrConf();

    double                eShift{0};
    double                eGain{1};
    int 		  reference{16};
    ReactionFragment::Ion ion;
    LabrLut               lut;
    // std::array<double, 5> timeExtr{};
    inline double         getEnergy(const double& val, const unsigned int& brd,
                                    const unsigned int& ch) const {
        return eGain * (lut.getEnergy(val, brd, ch)) + eShift;
    }
    inline double getQshort(const double& val, const unsigned int& brd,
                            const unsigned int& ch) const {
        return eGain * (lut.getQshort(val, brd, ch));
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
};
