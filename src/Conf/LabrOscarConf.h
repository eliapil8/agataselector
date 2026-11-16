#pragma once

#include "GammadetectorOscarConf.h"

class LabrOscarConf : public GammadetectorOscarConf {
  public:
    LabrOscarConf() = delete;

  protected:
    explicit LabrOscarConf(const std::string& mark);
};

class LabrOscarUpConf : public LabrOscarConf {
  public:
    LabrOscarUpConf() : LabrOscarConf("LABROSCARUP_CONF"){};
};

class LabrOscarDownConf : public LabrOscarConf {
  public:
    LabrOscarDownConf() : LabrOscarConf("LABROSCARDOWN_CONF"){};
};
