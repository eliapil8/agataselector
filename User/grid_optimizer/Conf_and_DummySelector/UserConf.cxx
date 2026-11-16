#include "UserConf.h"

UserConf::UserConf() : Conf("GRID_SEARCH_OPTIMIZATION") {
    parD.emplace(
        "PEAK_FIT_WIDTH_STRATEGY_VALUE",
        Conf::Property<double&>{peak_fit_width_strategy_value, "#",
                                "strategy FWxM -> define VAL (for FWFM: "
                                "VAL=5), for static width VAL=Y_val"});

    parS.emplace("PEAK_FIT_WIDTH_STRATEGY",
                 Conf::Property<std::string&>{
                     peak_fit_width_strategy, "#",
                     "strategy to fit the peak for width - dynamic FWxM  of "
                     "static width at given value Y"});
    parS.emplace("PEAK_FIT_FUNCTION",
                 Conf::Property<std::string&>{
                     peak_fit_function, "#",
                     "fit function to be used for peak fitting"});
    parS.emplace(
        "ENABLED_HISTOS",
        Conf::Property<std::string&>{enabled_histos, "#",
                                     "histograms to be produced by selector"});
    parS.emplace(
        "RESULT_FILE_PATH",
        Conf::Property<std::string&>{
            optiresult_file_path, "#",
            "path to store the finnal optimization result root file"});
    parS.emplace(
        "MATRIX_NAME",
        Conf::Property<std::string&>{
            matrix_name, "#", "name of the matrix to be used for fitting"});
    parS.emplace(
        "REDUCED_FILE_PATH",
        Conf::Property<std::string&>{reduced_file_path, "#",
                                     "Path to store reduced input files"});
    parSS.emplace("DATA_REDUCTION_CONDITION",
                  Conf::Property<std::string&>{
                      data_reduction_condition, "#",
                      "reduction condition applied on the trees from FEMUL, "
                      "e.i. BEFORE SELECTOR"});

    // parVS.emplace("DATA_REDUCTION_CONDITION",
    //               Conf::Property<std::vector<std::string>
    //               &>{data_reduction_condition_split, "#", "Path to store
    //               reduced input files"});

    parB.emplace(
        "RUN_PREPROCESSING",
        Conf::Property<bool&>{run_preprocessing, "#",
                              "option to skip pre-processing stage YES/NO"});
    parB.emplace(
        "RUN_SELECTOR_LOOP",
        Conf::Property<bool&>{run_selector_loop, "#",
                              "option to skip running selector stage YES/NO"});
    parB.emplace("RUN_ROOT_FILE_PROCESSING",
                 Conf::Property<bool&>{
                     run_root_file_processing, "#",
                     "option to skip root-file processing stage YES/NO"});
    parD.emplace(
        "FIT_PEAK_ENERGY",
        Conf::Property<double&>{
            fit_peak_energy, "keV",
            "rough energy of the peak to be fitted, used for initial guess"});
    parD.emplace("FIT_RANGE_START",
                 Conf::Property<double&>{
                     fit_range_start, "keV",
                     "bottom limit of the range in which to fit the peak"});
    parD.emplace("FIT_RANGE_END",
                 Conf::Property<double&>{
                     fit_range_end, "keV",
                     "top limit of the range in which to fit the peak"});
    parD.emplace("FIT_WEIGHT_WIDTH",
                 Conf::Property<double&>{
                     fit_weight_width, "#",
                     "weight of the width in the fitting cost function"});
    parD.emplace(
        "FIT_WEIGHT_POSITION",
        Conf::Property<double&>{fit_weight_position, "#",
                                "weight of the position in the fitting cost "
                                "function: abs(position - peak energy))"});
    parD.emplace("FIT_WEIGHT_VOLUME",
                 Conf::Property<double&>{
                     fit_weight_volume, "#",
                     "weight of the volume in the fitting cost function"});
    parD.emplace("FIT_WEIGHT_PAR3",
                 Conf::Property<double&>{
                     fit_weight_par3, "#",
                     "weight of the paramter 3 in the fitting cost function"});
    parD.emplace(
        "FIT_WEIGHT_PAR4",
        Conf::Property<double&>{
            fit_weight_par4, "#",
            "weight of the parameter 4 in the fitting cost function"});
    // for angular offsets
    parD.emplace("THETA_OFFSET_RANGE_START",
                 Conf::Property<double&>{
                     theta_offset_range_start, "deg",
                     "Bottom limit of range in which to vary theta angle"});
    parD.emplace("THETA_OFFSET_RANGE_END",
                 Conf::Property<double&>{
                     theta_offset_range_end, "deg",
                     "Top limit of range in which to vary theta angle"});
    parD.emplace(
        "THETA_STEP",
        Conf::Property<double&>{
            theta_step, "deg",
            "size of a theta step in deg, if defined range contain zero, the "
            "steps are "
            "always counted from 0 (included) until limit is reached"});
    parD.emplace("PHI_OFFSET_RANGE_START",
                 Conf::Property<double&>{
                     phi_offset_range_start, "deg",
                     "Bottom limit of range in which to vary theta angle"});
    parD.emplace("PHI_OFFSET_RANGE_END",
                 Conf::Property<double&>{
                     phi_offset_range_end, "deg",
                     "Top limit of range in which to vary theta angle"});
    parD.emplace(
        "PHI_STEP",
        Conf::Property<double&>{
            phi_step, "deg",
            "size of a phi step in deg, if defined range contain zero, the "
            "steps are "
            "always counted from 0 (included) until limit is reached"});

    parVD.emplace("THETA_CUSTOM_OFFSET",
                  Conf::Property<std::vector<double>&>{
                      theta_custom_offset, "#", "Concrete theta angles"});
    parVD.emplace("PHI_CUSTOM_OFFSET",
                  Conf::Property<std::vector<double>&>{phi_custom_offset, "#",
                                                       "Concrete phi angles"});

    // for carthesian offsets
    parD.emplace(
        "X_OFFSET_RANGE_START",
        Conf::Property<double&>{x_offset_range_start, "mm",
                                "Bottom limit in which to vary x position"});
    parD.emplace(
        "X_OFFSET_RANGE_END",
        Conf::Property<double&>{x_offset_range_end, "mm",
                                "Top limit in which to vary x position"});
    parD.emplace("X_STEP",
                 Conf::Property<double&>{x_step, "mm", "size of a step in x"});
    parD.emplace(
        "Y_OFFSET_RANGE_START",
        Conf::Property<double&>{y_offset_range_start, "mm",
                                "Bottom limit in which to vary y position"});
    parD.emplace(
        "Y_OFFSET_RANGE_END",
        Conf::Property<double&>{y_offset_range_end, "mm",
                                "Top limit in which to vary y position"});
    parD.emplace("Y_STEP",
                 Conf::Property<double&>{y_step, "mm", "size of a step in y"});
    parD.emplace(
        "Z_OFFSET_RANGE_START",
        Conf::Property<double&>{z_offset_range_start, "mm",
                                "Bottom limit in which to vary z position"});
    parD.emplace(
        "Z_OFFSET_RANGE_END",
        Conf::Property<double&>{z_offset_range_end, "mm",
                                "Top limit in which to vary z position"});
    parD.emplace("Z_STEP",
                 Conf::Property<double&>{z_step, "mm", "size of a step in z"});

    parVD.emplace("X_CUSTOM_OFFSET",
                  Conf::Property<std::vector<double>&>{
                      x_custom_offset, "#", "Concrete X positions"});
    parVD.emplace("Y_CUSTOM_OFFSET",
                  Conf::Property<std::vector<double>&>{
                      y_custom_offset, "#", "Concrete Y positions"});
    parVD.emplace("Z_CUSTOM_OFFSET",
                  Conf::Property<std::vector<double>&>{
                      z_custom_offset, "#", "Concrete Z positions"});
}
