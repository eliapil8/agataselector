#pragma once

#include "Conf.h"

class DetectorsPresentConf : public Conf {
  public:
    DetectorsPresentConf();

    bool Agata{false};
    bool Prisma{false};
    bool Spider{false};
    bool SauronFw{false};
    bool SauronBw{false};
    bool EXSSiDe{false};
    bool OscarUp{false};
    bool OscarDown{false};
    bool Labr{false};
    bool Dante{false};
    bool Euclides{false};
    bool Mcp{false};
    bool Monitor{false};
};
