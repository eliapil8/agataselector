#pragma once

#include "CoincidenceConf.h"
#include <cmath>
#include <string>
#include <vector>

class McpDanteConf : public CoincidenceConf {
  public:
    McpDanteConf();


  public:
    std::vector<std::string>               lutFiles{{"NONE"}};
};