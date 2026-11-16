#include "DetectorsPresentConf.h"

DetectorsPresentConf::DetectorsPresentConf() : Conf("DETECTORS_PRESENT") {
    parB.emplace("AGATA",
                 Conf::Property<bool&>{Agata, "#", "Agata is present YES/NO"});
    parB.emplace("PRISMA", Conf::Property<bool&>{Prisma, "#",
                                                 "Prisma is present YES/NO"});
    parB.emplace("SPIDER", Conf::Property<bool&>{Spider, "#",
                                                 "Spider is present YES/NO"});
    parB.emplace("SAURONFW",
                 Conf::Property<bool&>{SauronFw, "#",
                                       "Sauron Forward is present YES/NO"});
    parB.emplace("SAURONBW",
                 Conf::Property<bool&>{SauronBw, "#",
                                       "Sauron Backward is present YES/NO"});
    parB.emplace("EXSSIDE",
                 Conf::Property<bool&>{EXSSiDe, "#",
                                       "EXSSIDE Forward is present YES/NO"});
    parB.emplace("OSCARUP", Conf::Property<bool&>{
                                OscarUp, "#", "Oscar Up is present YES/NO"});
    parB.emplace(
        "OSCARDOWN",
        Conf::Property<bool&>{OscarDown, "#", "Oscar Down is present YES/NO"});
    parB.emplace("LABR",
                 Conf::Property<bool&>{Labr, "#", "Labr is present YES/NO"});
    parB.emplace("DANTE",
                 Conf::Property<bool&>{Dante, "#", "Dante is present YES/NO"});
    parB.emplace("EUCLIDES", Conf::Property<bool&>{
                                 Euclides, "#", "Euclides is present YES/NO"});
    parB.emplace("MCP",
                 Conf::Property<bool&>{Mcp, "#", "EXOTIC Mcps are present YES/NO"});
    parB.emplace("MONITOR",
                 Conf::Property<bool&>{Monitor, "#", "EXOTIC Monitors are present YES/NO"});
}
