#include "ReplayConf.h"

ReplayConf::ReplayConf() : Conf("REPLAY_CONF") {
    parS.emplace("IN_PATH",
                 Conf::Property<std::string&>{in_path, "#", "Input path"});
    parS.emplace("IN_SUB_PATH", Conf::Property<std::string&>{
                                    in_sub_path, "#", "Input sub path"});
    parS.emplace("OUT_PATH",
                 Conf::Property<std::string&>{out_path, "#", "Output path"});
    parS.emplace("CONF_PATH", Conf::Property<std::string&>{
                                  conf_path, "#", "Replay conf folder path"});
    parS.emplace("REPLAY_DIR_PATTERN",
                 Conf::Property<std::string&>{replay_dir_pattern, "#",
                                              "Replay directory pattern"});
    parS.emplace("IN_FILE_PATTERN",
                 Conf::Property<std::string&>{in_file_pattern, "#",
                                              "Input file pattern"});
    parS.emplace("OUT_FILE_PATTERN",
                 Conf::Property<std::string&>{out_file_pattern, "#",
                                              "Output file pattern"});
    parS.emplace("SUM_FILE_PATTERN",
                 Conf::Property<std::string&>{sum_file_pattern, "#",
                                              "Hadded file pattern"});
    parS.emplace("SUM_ALL_PATTERN",
                 Conf::Property<std::string&>{sum_all_pattern, "#",
                                              "Sum of all hadded files pattern"});

    parS.emplace("TREE_NAME", Conf::Property<std::string&>{tree_name, "#",
                                                           "Input tree name"});
    parS.emplace("ENABLED_HISTOS",
                 Conf::Property<std::string&>{
                     enabled_histo_file_name, "#",
                     "File name with list of enabled histograms"});
}
