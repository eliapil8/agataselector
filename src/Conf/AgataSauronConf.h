#pragma once

#include "GammadetectorSauronConf.h"

class AgataSauronConf : public GammadetectorSauronConf {
  public:
    AgataSauronConf() = delete;

  protected:
    explicit AgataSauronConf(const std::string& mark);
};

class AgataSauronFwConf : public AgataSauronConf {
  public:
    AgataSauronFwConf() : AgataSauronConf("AGATASAURONFW_CONF"){};
};

class AgataSauronBwConf : public AgataSauronConf {
  public:
    AgataSauronBwConf() : AgataSauronConf("AGATASAURONBW_CONF"){};
};
