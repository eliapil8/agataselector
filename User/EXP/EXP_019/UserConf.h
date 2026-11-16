#pragma once

#include "Conf.h"

class UserConf : public Conf {
  public:
    UserConf();

  public:
    double threshold_rate{5500};
};
