#include "UserConf.h"

UserConf::UserConf() : Conf("USER_CONF") {
    parD.emplace("THR_ANGLE",
                 Conf::Property<double&>{thrAngle, "degrees",
                                         "Angle threshold ...."});
}
