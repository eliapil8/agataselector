#pragma once

#include <array>

#include "DanteLut.h"
#include "DetectorConf.h"
#include "ReactionFragment.h"

class DanteConf : public DetectorConf {
  public:
    DanteConf();

    ReactionFragment::Ion ion;
    ReactionFragment::Ion ion2;
    bool                  mandatoryTime{true};
    bool                  mandatoryX{true};
    bool                  mandatoryY{true};
    bool                  mandatoryTOF{false};
    double                phiCoinc{10*UNITS::deg};
    DanteLut              lut;
    inline std::pair<double, double>
    getThr(const unsigned int& brd, const unsigned int& ch) const {
        return lut.getThr(brd, ch);
    }
    inline unsigned int
    getDanteId(const unsigned int& brd, const unsigned int& ch) const {
        return lut.getDanteId(brd, ch);
    }
    inline std::pair<unsigned int, unsigned int>
    getBrdChanFromId(const unsigned int&        danteId,
                     const DanteLut::DanteType& type) const {
        return lut.getBrdChanFromId(danteId, type);
    }
    inline unsigned int
    getDanteType(const unsigned int& brd, const unsigned int& ch) const {
        return lut.getDanteType(brd, ch);
    }
    inline TVector3 getPosition(const double& x, const double& y,
                                const unsigned int& danteId) const {
        TVector3 vec = lut.getPos(x, y, danteId);
        vec += TVector3(xShift, yShift, zShift);
        vec.RotateZ(phi);
        vec.RotateX(thetaX);
        vec.RotateY(thetaY);

        return vec;
    }
    inline double getAvgX(const unsigned int& danteId) const {
        return lut.getAvgX(danteId);
    }
    inline double getAvgY(const unsigned int& danteId) const {
        return lut.getAvgY(danteId);
    }
    inline double
    getTime(const double& val, const unsigned int& brd, const unsigned int& chan) const {
        return lut.getTime(val, brd, chan);
    }
    inline double
    getTof(const double& val, const unsigned int& danteId) const {
        return lut.getTof(val, danteId);
    }
    inline bool hasTof(const unsigned int& danteId) const {
        return lut.hasTof(danteId);
    }
    inline bool tofType(const unsigned int& danteId) const {
        return lut.tofType(danteId);
    }
    std::vector<unsigned int> getAllDanteId() const {
        return lut.getAllDanteId();
    };
};
