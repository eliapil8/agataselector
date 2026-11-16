#pragma once

#include <vector>

#include "CoincidenceConf.h"
#include "Units.h"

class PrismaDanteConf : public CoincidenceConf {
  public:
    PrismaDanteConf();

    double max_theta_discr{90 * UNITS::deg};
    double max_phi_discr{90 * UNITS::deg};
};
