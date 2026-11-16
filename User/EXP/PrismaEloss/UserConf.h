#pragma once

#include "Conf.h"

class UserConf : public Conf {
  public:
    UserConf();

  public:
    bool eloss{false};
    std::vector<std::pair<int, int>> a_ranges;
    std::vector<std::pair<int, int>> z_ranges;
    double min_t_ica{225 * UNITS::mm};
    double max_t_ica{265 * UNITS::mm};
    double step_t_ica{5 * UNITS::mm};
    double min_beta{0.05};
    double max_beta{0.15};
    int n_beta{5};
    std::string el_tables_path{"/EnergyLoss"};
    std::string out_el_file{"eloss_user.dat"};
    std::string ic_el_name{"CF4_70mbar"};
};