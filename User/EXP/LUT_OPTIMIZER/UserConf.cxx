#include "UserConf.h"

UserConf::UserConf() : Conf("USER_CONF") {
    parVS.emplace("HISTO",
                 Conf::Property<std::vector<std::string>&>{histoList, "#",
                                         "Histogram to be optimized"});
    parS.emplace("DETECTOR",
                  Conf::Property<std::string&>{detector, "#", "Name of detector to optimize"});
}
