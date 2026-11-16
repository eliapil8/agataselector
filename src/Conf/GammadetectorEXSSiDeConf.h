#pragma once

#include "CoincidenceConf.h"
#include <cmath>
#include <string>
#include <vector>

class GammadetectorEXSSiDeConf : public CoincidenceConf {
  public:
    GammadetectorEXSSiDeConf() = delete;

  protected:
    explicit GammadetectorEXSSiDeConf(const std::string& mark);

  public:
    std::vector<std::string>               lutFiles{{"NONE"}};
    std::vector<std::pair<double, double>> exValues{
        {0, 1}
    };
};
