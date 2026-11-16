#pragma once

#include "CoincidenceConf.h"
#include <vector>

class GammadetectorEuclidesConf : public CoincidenceConf {
  public:
    GammadetectorEuclidesConf() = delete;

  protected:
    explicit GammadetectorEuclidesConf(const std::string& mark);

  public:
    std::vector<std::pair<double, double>> exValues{
        {0, 1}
    };
};
