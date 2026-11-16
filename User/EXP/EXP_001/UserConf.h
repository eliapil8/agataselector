#pragma once

#include <vector>

#include "Conf.h"


class UserConf : public Conf {
  public:
    UserConf();

  public:
  double example{0};
  std::vector<std::pair<double, double>> ex_gates;

  ULong64_t offTS;
  
};
