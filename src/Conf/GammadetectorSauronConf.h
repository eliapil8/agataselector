#pragma once

#include "CoincidenceConf.h"
#include <cmath>
#include <string>
#include <vector>

class GammadetectorSauronConf : public CoincidenceConf {
  public:
    GammadetectorSauronConf() = delete;

  protected:
    explicit GammadetectorSauronConf(const std::string& mark);

  public:
    std::vector<std::string>               lutFiles{{"NONE"}};
    std::vector<std::pair<double, double>> exValues{
        {0, 1}
    };
};
