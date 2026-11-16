#pragma once

#include "CoincidenceConf.h"
#include <cmath>
#include <string>
#include <vector>

class McpMonitorConf : public CoincidenceConf {
  public:
    McpMonitorConf();

    bool                                  enablecutetof{false};
    std::vector<std::pair<int, int>>      cut_etof_ids{{0,0}};
  
  public:
    std::vector<std::string>               lutFiles{{"NONE"}};
};