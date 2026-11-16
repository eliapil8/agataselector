#pragma once

#include "GammadetectorConf.h"

class AgataConf : public GammadetectorConf {
  public:
    AgataConf();

    int    binWidth{1};
    double minFOMThreshold{0};
    double maxFOMThreshold{2};
    int trackType{-1};

  protected:
};
