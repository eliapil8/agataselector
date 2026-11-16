#include "GammadetectorEuclidesConf.h"

GammadetectorEuclidesConf::GammadetectorEuclidesConf(const std::string& mark)
    : CoincidenceConf(mark) {
    parVP.emplace("EX_VALUES",
                  Conf::Property<std::vector<std::pair<double, double>>&>{
                      exValues, "MeV", "Excitation energy gates"});
}
