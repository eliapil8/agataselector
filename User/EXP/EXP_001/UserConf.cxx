#include "UserConf.h"

UserConf::UserConf() : Conf("USER_CONF") {
    parD.emplace("EXAMPLE",
                 Conf::Property<double&>{example, "mm",
                                         "Example of configuration value"});
    parVP.emplace("EX_GATE",
		  Conf::Property<std::vector<std::pair<double, double>>&>{ex_gates,
		    "#", "Gate on excitation energy"});

    parULL.emplace("TS_OFF",
		   Conf::Property<unsigned long long&>{offTS,
		     "#", "offset to initial TS"});
}
