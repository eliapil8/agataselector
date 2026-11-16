#pragma once

#include "Conf.h"

class UserConf : public Conf {
  public:
    UserConf();

  public:
    //    double example{0};
    std::string IsotopeName;
    double      minE1{-1};
    double      maxE1{-1};
    double      minE2{-1};
    double      maxE2{-1};
    double      minT{0};
    double      maxT{0};
    bool        crysSpectra{false};
};