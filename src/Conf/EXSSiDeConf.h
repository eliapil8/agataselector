#pragma once

#include <array>

#include "DetectorConf.h"
#include "ReactionFragment.h"
#include "EXSSiDeLut.h"


/*Naming scheme for channels:
        -2mm-           +y (ceiling)
    |   |   |   |   |
    -----------------
    |   |   |   |   |
    -----------------
    |   |   |   |   |
    -----------------   beam +z
    |   |   |   |   |
    -----------------
    |   |   |   |   |
    -----------------
    |   |   |   |   |
`
  NSIDECH00 -> N side, Channel 0-31
  PSIDECH32 -> P side, Channel 32-63

*/
class EXSSiDeConf : public DetectorConf {
  
  public:
    EXSSiDeConf();

    bool                  pSidePriority{false};
    bool                  singleSideDetector{false};
    double                distanceFromTarget{6994. * UNITS::mm};
    double                eShift{0};
    unsigned int          avgx{48};
    unsigned int          avgy{16};
    unsigned int          refch{15};
    double                eGain{1};
    double                minE{2000};
    double                eneWindowLeft;
    double                eneWindowRight;
    double                exssidebinWidth{0.1};
    double                exssideEmax{10000 * UNITS::MeV};
    double                deadlayerThickness{0 * UNITS::ug / UNITS::cm2};
    std::string           deadlayerMaterial{"none"};
    double                absorberThickness{0 * UNITS::ug / UNITS::cm2};
    std::string           absorberMaterial{"none"};
    ReactionFragment::Ion ion;
    EXSSiDeLut             lut;
    std::vector<std::pair<int, int>> brokenChannels;
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
    getStrip(const unsigned int& ch) const {
        return lut.getStripNr(ch);
    }
    inline TVector3 
    getPosition(const unsigned int &ch1, const unsigned int &ch2) const {
        TVector3 pos = lut.getPos(ch1,ch2);
        pos += TVector3(xShift, yShift, zShift);
        pos.RotateZ(phi);
        pos.RotateX(thetaX);
        pos.RotateY(thetaY);
        return pos;
    }
    inline unsigned int
    getAvgX() const {
        if(lut.settings.verbosity_level>4){
            std::cout<<"Average X position in channel [0:63] = "<<avgx<<std::endl;
        }
        return avgx;
    }
    inline unsigned int
    getAvgY() const {
        if(lut.settings.verbosity_level>4){
            std::cout<<"Average Y position in channel [0:63] = "<<avgy<<std::endl;
        }
        return avgy;
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
