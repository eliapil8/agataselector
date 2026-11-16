#include "DanteConf.h"

DanteConf::DanteConf() : DetectorConf("DANTE_CONF") {
    lutFile  = "LUT_DANTE.dat";
    cfd_unit = 1 * UNITS::picosecond;

    parION.emplace("ION", Conf::Property<ReactionFragment::Ion&>{
                              ion, "#", "Ion assumed in dante"});
    parION.emplace("ION2", Conf::Property<ReactionFragment::Ion&>{
                              ion2, "#", "Ion assumed in dante-dante coincidence as the fast one"});
    parB.emplace("MANDATORY_T", Conf::Property<bool&>{
                                    mandatoryTime, "#",
                                    "request time for a valid dante event"});
    parB.emplace("MANDATORY_X",
                 Conf::Property<bool&>{mandatoryX, "#",
                                       "request X for a valid dante event"});
    parB.emplace("MANDATORY_Y",
                 Conf::Property<bool&>{mandatoryY, "#",
                                       "request Y for a valid dante event"});
    parB.emplace("MANDATORY_TOF",
                 Conf::Property<bool&>{mandatoryTOF, "#",
                                       "request TOF for a valid dante event"});
    parD.emplace("PHI_COINC",
                 Conf::Property<double&>{phiCoinc, "deg",
                                       "gate of coincidence in phi for dante-dante binary reaction"});
}
