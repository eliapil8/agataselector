#pragma once

#include <array>

#include "DetectorConf.h"
#include "ReactionFragment.h"
#include "MonitorLut.h"

class MonitorConf : public DetectorConf {
  public:
    MonitorConf();

    double                eShift{0};
    double                eGain{1};
    //bool                  keepHighEneEvent{true};
    double                monitorbinWidth{1};
    double                monitorEmax{400 * UNITS::MeV};
    double                deadlayer_thickness{0 * UNITS::ug / UNITS::cm2};
    std::string           deadlayer_material{"27-13"};
    ReactionFragment::Ion ion;
    MonitorLut             lut;
    
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
    std::vector<unsigned int> getAllMonitorId() const {
        return lut.getAllMonitorId();
    };
    inline std::pair<double, double>
    getThr(const unsigned int& brd, const unsigned int& ch) const {
        return lut.getThr(brd, ch);
    }
    inline std::string
    getName(const unsigned int& brd, const unsigned int& ch) const {
        return lut.getName(brd, ch);
    }
};
