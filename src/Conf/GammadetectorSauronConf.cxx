#include "GammadetectorSauronConf.h"

GammadetectorSauronConf::GammadetectorSauronConf(const std::string& mark)
    : CoincidenceConf(mark) {
    parVP.emplace("EX_VALUES",
                  Conf::Property<std::vector<std::pair<double, double>>&>{
                      exValues, "MeV",
                      "Excitation energy gates, first value is the min, "
                      "second value is the max"});
}
