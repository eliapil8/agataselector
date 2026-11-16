#pragma once

#include <array>
#include <unordered_map>

#include "DetectorConf.h"
#include "ReactionFragment.h"
#include "SpiderLut.h"

class SpiderConf : public DetectorConf {
  public:
    SpiderConf();

    double                eShift{0};
    double                eGain{1.};
    double                exShift{0};
    double                exGain{1.};
    double                exQuad{0};
    bool                  keepHighEneEvent{true};
    int                   Ebins{1000};
    double                Emax{200 * UNITS::MeV};
    double                Emin{0 * UNITS::MeV};
    double                Exmax{90 * UNITS::MeV};
    double                Exmin{-10 * UNITS::MeV};
    int                   Exbins{1000};
    double                cosAmpli{0.};
    double                cosPhase{0.};
    double                cosPeriod{1.};
    double                deadlayer_thickness{0 * UNITS::ug / UNITS::cm2};
    std::string           deadlayer_material{"27-13"};
    ReactionFragment::Ion ion;
    SpiderLut             lut;
    std::string           ethetacutPath{"/CUT/SPIDER/ETheta/"};
    std::string           exthetaPath{"/CUT/SPIDER/ExTheta/"};
    std::string           exphiPath{"/CUT/SPIDER/ExPhi/"};
    std::string           exidPath{"/CUT/SPIDER/ExID/"};
    std::vector<std::pair<int, int>> broken_channels{
        {0, 0}
    };
  std::unordered_map<int, int>      aliasId{};
  
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
};
