#pragma once

#include "CoincidenceConf.h"
#include <cmath>
#include <string>
#include <vector>

class McpEXSSiDeConf : public CoincidenceConf {
  public:
    McpEXSSiDeConf();

  bool    debug{false};
  double  sourcex{0.};
  double  sourcey{0.};
  double  sourcez{-7110.};
  
  public:
    std::vector<std::string>               lutFiles{{"NONE"}};
  //  std::vector<std::pair<double, double>> exValues{{0, 1}};
};