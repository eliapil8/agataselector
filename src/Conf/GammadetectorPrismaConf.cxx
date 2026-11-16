#include "GammadetectorPrismaConf.h"

GammadetectorPrismaConf::GammadetectorPrismaConf(const std::string& mark)
    : CoincidenceConf(mark) {
    parVP.emplace("EX_VALUES",
                  Conf::Property<std::vector<std::pair<double, double>>&>{
                      exValues, "MeV",
                      "Excitation energy gates, first value is the min, "
                      "second value is the max"});
    parD.emplace("LIFETIME_ION",
                 Conf::Property<double&>{lifetime_ion, "ns",
                                         "Mean lifetime of populated excited state for detected ion to compute displacement from target"});
    parD.emplace("LIFETIME_IONBP",
                 Conf::Property<double&>{lifetime_ionBP, "ns",
                                         "Mean lifetime of populated excited state for binary partner to compute displacement from target"});
}
