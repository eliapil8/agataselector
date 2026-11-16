#pragma once

#include "CoincidenceConf.h"
#include <cmath>
#include <string>
#include <vector>

class McpSauronConf : public CoincidenceConf {
  public:
    McpSauronConf() = delete;

  protected:
    explicit McpSauronConf(const std::string& mark);

  public:
    std::vector<std::string>               lutFiles{{"NONE"}};
};


class McpSauronFwConf : public McpSauronConf {
  public:
    McpSauronFwConf() : McpSauronConf("MCPSAURONFW_CONF"){};
};

class McpSauronBwConf : public McpSauronConf {
  public:
    McpSauronBwConf() : McpSauronConf("MCPSAURONBW_CONF"){};
};
