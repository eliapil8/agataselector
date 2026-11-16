#include "AgataConf.h"

AgataConf::AgataConf() : GammadetectorConf("AGATA_CONF") {
    spectraType = TRACKED;
    cfd_unit    = 10 * UNITS::ns;

    parD.emplace(
        "MIN_FOM_THR",
        Conf::Property<double&>{
            minFOMThreshold, "#",
            "Min FOM to accept a tracked gamma (0-1)"});
    parD.emplace(
        "MIN_FOM_THR",
        Conf::Property<double&>{
            maxFOMThreshold, "#",
            "Max FOM to accept a tracked gamma (0-1.3), 1.2 are single interaction events"});
}
