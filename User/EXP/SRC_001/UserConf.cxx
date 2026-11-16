#include "UserConf.h"

UserConf::UserConf() : Conf("USER_CONF") {
    parD.emplace("EXAMPLE",
                 Conf::Property<double&>{example, "mm",
                                         "Example of configuration value"});

    parD.emplace("EXAMPLE",
                 Conf::Property<double&>{example, "mm",
                                         "Example of configuration value"});
}
