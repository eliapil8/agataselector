#pragma once

#include "Conf.h"

class UserConf : public Conf {
  public:
    UserConf();

  public:
    double gamma_tof_offset{0};
    double gamma_e_max{0};
};
