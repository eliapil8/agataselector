#include "GammadetectorOscarConf.h"

GammadetectorOscarConf::GammadetectorOscarConf(const std::string& mark)
    : CoincidenceConf(mark) {
    parD.emplace(
        "BETA_AVG_MAG",
        Conf::Property<double&>{
            betaAvgMag, "#",
            "Average beta magnitude Doppler correction for coincidence"});
    parD.emplace("BETA_AVG_THETA",
                 Conf::Property<double&>{
                     betaAvgTheta, "deg",
                     "Average beta theta Doppler correction for coincidence"});
    parD.emplace("BETA_AVG_PHI",
                 Conf::Property<double&>{
                     betaAvgPhi, "deg",
                     "Average beta phi Doppler correction for coincidence"});
    parVP.emplace("EX_VALUES",
                  Conf::Property<std::vector<std::pair<double, double>>&>{
                      exValues, "MeV",
                      "Excitation energy gates, first value is the min, "
                      "second value is the max"});
}
