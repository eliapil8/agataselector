#pragma once

#include "CoincidenceConf.h"
#include <cmath>
#include <string>
#include <vector>

class GammadetectorOscarConf : public CoincidenceConf {
  public:
    GammadetectorOscarConf() = delete;

  protected:
    explicit GammadetectorOscarConf(const std::string& mark);

  public:
    double                                 betaAvgMag{0.1};
    double                                 betaAvgTheta{20 * UNITS::deg};
    double                                 betaAvgPhi{180 * UNITS::deg};
    TVector3                               betaAvg{0, 0, 0};
    std::vector<std::string>               lutFiles{{"NONE"}};
    std::vector<std::pair<double, double>> exValues{
        {0, 1}
    };
};
