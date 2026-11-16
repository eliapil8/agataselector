#include "UserConf.h"

UserConf::UserConf() : Conf("USER_CONF") {
    parD.emplace("GAMMA_TOF_OFFSET",
                 Conf::Property<double&>{gamma_tof_offset, "ns",
                                         "tof offset for gamma DC optimization"});
    parD.emplace("GAMMA_E_MAX",
                 Conf::Property<double&>{gamma_e_max, "keV",
                                         "Gamma Max Energy"});
}
