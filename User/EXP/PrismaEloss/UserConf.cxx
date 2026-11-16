#include "UserConf.h"

UserConf::UserConf() : Conf("USER_CONF") {
        parB.emplace("ELOSS",
                Conf::Property<bool&>{eloss, "#",
                                       "Calculate E loss"});
        parVPI.emplace("A_RANGE",
                Conf::Property<std::vector<std::pair<int, int>>&>{
                       a_ranges, "#", "A range for which to calculate energy loss"});
        parVPI.emplace("Z_RANGE",
                Conf::Property<std::vector<std::pair<int, int>>&>{
                       z_ranges, "#", "Z range for which to calculate energy loss"});
        parD.emplace("MIN_THICKNESS_IC_A",
                Conf::Property<double&>{min_t_ica, "mm",
                                       "Min thickness of IC gas A"});
        parD.emplace("MAX_THICKNESS_IC_A",
                Conf::Property<double&>{max_t_ica, "mm",
                                       "Max thickness of IC gas A"});
        parD.emplace("STEP_THICKNESS_IC_A",
                Conf::Property<double&>{step_t_ica, "mm",
                                       "Step of thickness of IC gas A for scan"});
        parD.emplace("MIN_BETA",
                Conf::Property<double&>{min_beta, "#",
                                       "Min beta of ions"});
        parD.emplace("MAX_BETA",
                Conf::Property<double&>{max_beta, "#",
                                       "Max beta of ions"});
        parI.emplace("N_STEPS",
                Conf::Property<int&>{n_beta, "#",
                                       "Number of beta values"});
        parS.emplace("EL_PATH",
                Conf::Property<std::string&>{el_tables_path, "#",
                                       "Path to energy loss tables"});
        parS.emplace("OUT_EL_FILE",
                Conf::Property<std::string&>{out_el_file, "#",
                                       "Output path for energy loss calculation"});
        parS.emplace("IC_EL_NAME",
                Conf::Property<std::string&>{ic_el_name, "#",
                                       "Name of the energy loss tablefor the ic gas"});

    }
