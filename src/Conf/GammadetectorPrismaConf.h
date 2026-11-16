#pragma once

#include "CoincidenceConf.h"

class GammadetectorPrismaConf : public CoincidenceConf {
  public:
    GammadetectorPrismaConf() = delete;

  protected:
    explicit GammadetectorPrismaConf(const std::string& mark);

  public:
    std::vector<std::pair<double, double>> exValues{
        {0, 1}
    };
    double lifetime_ion{0.};
    double lifetime_ionBP{0.};
};
