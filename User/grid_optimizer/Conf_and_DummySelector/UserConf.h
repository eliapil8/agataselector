#pragma once

#include "Conf.h"
#include <algorithm>
#include <limits>
#include <string>

class UserConf : public Conf {
  public:
    UserConf();

  private:
    std::vector<double> fTheta_angles;
    std::vector<double> fPhi_angles;
    int                 fTheta_index;
    int                 fPhi_index;

  public:
    double theta_offset_range_start{std::numeric_limits<double>::infinity()};
    double theta_offset_range_end{std::numeric_limits<double>::infinity()};
    double theta_step{std::numeric_limits<double>::infinity()};
    double phi_offset_range_start{std::numeric_limits<double>::infinity()};
    double phi_offset_range_end{std::numeric_limits<double>::infinity()};
    double phi_step{std::numeric_limits<double>::infinity()};

    std::vector<double> theta_custom_offset;
    std::vector<double> phi_custom_offset;

    double x_offset_range_start{std::numeric_limits<double>::infinity()};
    double y_offset_range_start{std::numeric_limits<double>::infinity()};
    double z_offset_range_start{std::numeric_limits<double>::infinity()};
    double x_offset_range_end{std::numeric_limits<double>::infinity()};
    double y_offset_range_end{std::numeric_limits<double>::infinity()};
    double z_offset_range_end{std::numeric_limits<double>::infinity()};

    double x_step{std::numeric_limits<double>::infinity()};
    double y_step{std::numeric_limits<double>::infinity()};
    double z_step{std::numeric_limits<double>::infinity()};

    std::vector<double> x_custom_offset;
    std::vector<double> y_custom_offset;
    std::vector<double> z_custom_offset;

    bool                run_preprocessing{true};
    bool                run_selector_loop{true};
    bool                run_root_file_processing{true};
    std::string         reduced_file_path{""};
    std::string         data_reduction_condition{""};
    std::string         matrix_name;

    std::string         optiresult_file_path{""};
    std::string         enabled_histos;

    std::string         peak_fit_function{"SkewedGaussian"};
    std::string         peak_fit_width_strategy{"FWxM"};
    double              peak_fit_width_strategy_value{5};

    double              fit_peak_energy{0.0};
    double              fit_range_start{0.0};
    double              fit_range_end{0.0};

    double              fit_weight_width{1.};
    double              fit_weight_position{0.};
    double              fit_weight_volume{0.};
    double              fit_weight_par3{0.};
    double              fit_weight_par4{0.};
};