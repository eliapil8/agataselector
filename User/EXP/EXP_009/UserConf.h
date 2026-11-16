#pragma once

#include "Conf.h"

class UserConf : public Conf {
  public:
    UserConf();

  public:
    bool create_aoqhisto{false};
    bool create_agatarunhisto{false};
    bool create_prismarunhisto{false};
    bool create_agataprismarunhisto{false};
    bool create_agatatimehisto{false};
    bool create_mixhisto{false};
    bool eloss{false};
};