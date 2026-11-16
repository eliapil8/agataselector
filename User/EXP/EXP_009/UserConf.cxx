#include "UserConf.h"

UserConf::UserConf() : Conf("USER_CONF") {
    parB.emplace("CREATE_AOQHISTO",
                 Conf::Property<bool&>{create_aoqhisto, "#",
                                       "Draw histograms AoverQ vs variables"});
    parB.emplace("AGATARUNHISTO",
                 Conf::Property<bool&>{create_agatarunhisto, "#",
                                       "Draw histograms Agata vs run number"});
    parB.emplace(
        "PRISMARUNHISTO",
        Conf::Property<bool&>{create_prismarunhisto, "#",
                              "Draw histograms Prisma vs run number"});
    parB.emplace(
        "AGATAPRISMARUNHISTO",
        Conf::Property<bool&>{create_agataprismarunhisto, "#",
                              "Draw histograms Agata-Prisma vs run number"});
    parB.emplace("AGATATIMEHISTO",
                 Conf::Property<bool&>{create_agatatimehisto, "#",
                                       "Draw histograms Agata vs time"});
    parB.emplace("MIXHISTO",
                 Conf::Property<bool&>{create_mixhisto, "#",
                                       "Draw miscellaneous histograms"});
    parB.emplace("ELOSS",
                 Conf::Property<bool&>{eloss, "#",
                                       "Calculate E loss"});
}
