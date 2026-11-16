#pragma once

#include "Conf.h"
#include "Units.h"

#include <TVector3.h>

class DetectorConf : public Conf {
  public:
    DetectorConf() = delete;

  protected:
    explicit DetectorConf(const std::string& mark);

  public:
    int         windowLeft{-1};
    int         windowRight{0};
    int         windowLeftLeftBkg{};
    int         windowRightLeftBkg{};
    int         windowLeftRightBkg{};
    int         windowRightRightBkg{};
    double      xShift{0};
    double      yShift{0};
    double      zShift{0};
    double      phi{0};
    double      angle{0};
    double      thetaX{0};
    double      thetaY{0};
    double      cfd_unit{0 * UNITS::nanosecond};
    double      time_unit{10 * UNITS::nanosecond};
    double      betaAvgMag{0.1};
    double      betaAvgTheta{20 * UNITS::deg};
    double      betaAvgPhi{180 * UNITS::deg};
    bool        enable_histos{true};
    bool        enable_treeFill{false};
    TVector3    betaAvg{0, 0, 0};
    std::string lutFile{};
};
