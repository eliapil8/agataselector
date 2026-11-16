#pragma once

#include "Conf.h"

class UserConf : public Conf {
  public:
    UserConf();

  public:
    std::vector<std::string> histoList{""};
    std::string detector{"SPIDER"};
};