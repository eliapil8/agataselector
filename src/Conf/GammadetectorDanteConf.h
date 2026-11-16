#pragma once

#include "CoincidenceConf.h"
#include <vector>

class GammadetectorDanteConf : public CoincidenceConf {
  public:
    GammadetectorDanteConf() = delete;

  protected:
    explicit GammadetectorDanteConf(const std::string& mark);
};
