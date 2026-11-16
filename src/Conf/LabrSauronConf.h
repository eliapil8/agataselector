#pragma once

#include "GammadetectorSauronConf.h"

class LabrSauronConf : public GammadetectorSauronConf {
  public:
    LabrSauronConf() = delete;

  protected:
    explicit LabrSauronConf(const std::string& mark);
};

class LabrSauronFwConf : public LabrSauronConf {
  public:
    LabrSauronFwConf() : LabrSauronConf("LABRSAURONFW_CONF"){};
};

class LabrSauronBwConf : public LabrSauronConf {
  public:
    LabrSauronBwConf() : LabrSauronConf("LABRSAURONBW_CONF"){};
};
