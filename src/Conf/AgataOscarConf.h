#pragma once

#include "GammadetectorOscarConf.h"

class AgataOscarConf : public GammadetectorOscarConf {
  public:
    AgataOscarConf() = delete;

  protected:
    explicit AgataOscarConf(const std::string& mark);
};

class AgataOscarUpConf : public AgataOscarConf {
  public:
    AgataOscarUpConf() : AgataOscarConf("AGATAOSCARUP_CONF"){};
};

class AgataOscarDownConf : public AgataOscarConf {
  public:
    AgataOscarDownConf() : AgataOscarConf("AGATAOSCARDOWN_CONF"){};
};
