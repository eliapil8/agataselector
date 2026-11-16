#pragma once

#include <array>

#include "DetectorConf.h"
#include "ReactionFragment.h"
#include "SauronLut.h"
/*
     FRAME OF REFERENCE
              +y (ceiling)
           ---------
          /  -----  \
         /  /     \  \
        |  | beam  |  |
     +x |  |  +z   |  |
         \  \     /  /
          \  -----  /
           ---------
*/
class SauronConf : public DetectorConf {
  public:
    SauronConf() = delete;

  protected:
    explicit SauronConf(const std::string& mark);

  public:
    bool                  pSidePriority{false};
    bool                  singleSideDetector{false};
    bool                  pidHistograms{false};
    bool                  pidFromPside{true};
    bool                  pidMatch{false};
    double                distanceFromTarget{100. * UNITS::mm};
    double                eShift{0};
    double                eGain{1};
    double                eneWindowLeft{-100 * UNITS::keV};
    double                eneWindowRight{100 * UNITS::keV};
    double                sauronbinWidth{.1};
    double                sauronEmax{200 * UNITS::MeV};
    double                sauronExmax{90 * UNITS::MeV};
    double                sauronExbinWidth{0.1};
    double                deadlayerThickness{0 * UNITS::ug / UNITS::cm2};
    std::string           deadlayerMaterial{"27-13"};
    double                absorberThickness{0 * UNITS::ug / UNITS::cm2};
    std::string           absorberMaterial{"none"};
    ReactionFragment::Ion ion;
    SauronLut             lut;
    std::vector<std::pair<int, int>> brokenChannels{
        {0, 0}
    };
    // std::array<double, 5> timeExtr{};
    inline double getEnergy(const double& val, const unsigned int& brd,
                            const unsigned int& ch) const {
        return eGain * (lut.getEnergy(val, brd, ch)) + eShift;
    }
    inline double getTime(const double& val, const unsigned int& brd,
                          const unsigned int& ch) const {
        return lut.getTime(val, brd, ch);
    }
    inline std::pair<double, double>
    getThr(const unsigned int& brd, const unsigned int& ch) const {
        return lut.getThr(brd, ch);
    }
    inline std::string
    getName(const unsigned int& brd, const unsigned int& ch) const {
        return lut.getName(brd, ch);
    }
    inline int
    getSegment(const unsigned int& brd, const unsigned int& ch) const {
        return lut.getSegment(brd, ch);
    }
    inline double
    getRadius(const unsigned int& brd, const unsigned int& ch) const {
        return lut.getRadius(brd, ch);
    }
    inline double
    getPhi(const unsigned int& brd, const unsigned int& ch) const {
        return lut.getPhi(brd, ch);
    }
    inline bool
    isPside(const unsigned int& brd, const unsigned int& ch) const {
        return lut.isPside(brd, ch);
    }
    inline bool
    isNside(const unsigned int& brd, const unsigned int& ch) const {
        return lut.isNside(brd, ch);
    }
    inline bool
    isPresent(const unsigned int& brd, const unsigned int& ch) const {
        return lut.isPresent(brd, ch);
    }
};

class SauronFwConf : public SauronConf {
  public:
    SauronFwConf() : SauronConf("SAURONFW_CONF"){};
};

class SauronBwConf : public SauronConf {
  public:
    SauronBwConf() : SauronConf("SAURONBW_CONF"){};
};
