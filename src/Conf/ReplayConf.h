#pragma once

#include "Conf.h"

class ReplayConf : public Conf {
  public:
    ReplayConf();

    std::string in_path{"./Data"};
    std::string in_sub_path{"/Out/Analysis"};
    std::string out_path{"./Out"};
    std::string conf_path{"./Conf"};
    std::string replay_dir_pattern{"run_"};
    std::string in_file_pattern{"Tree_"};
    std::string out_file_pattern{"run_"};
    std::string sum_all_pattern{"runs"};
    std::string sum_file_pattern{"sum"};
    std::string tree_name{"TreeMaster"};
    std::string enabled_histo_file_name{"enabled_histos.conf"};
};
