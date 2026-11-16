#include "UserConf.h"

UserConf::UserConf() : Conf("USER_CONF") {
    parD.emplace("RATE_THR",
                 Conf::Property<double&>{
                     threshold_rate, "#",
                     "Threshold to distinguish decay and irradiation"});
}
