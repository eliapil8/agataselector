#include "UserConf.h"

UserConf::UserConf() : Conf("USER_CONF") {
    // parD.emplace("EXAMPLE",
    //              Conf::Property<double&>{example, "mm",
    //                                      "Example of configuration value"});
    parS.emplace("ISOTOPE_NAME",
                 Conf::Property<std::string&>{IsotopeName, "#",
                                              "Name of the analyzed isotope"});
    parD.emplace(
        "MIN_E1",
        Conf::Property<double&>{
            minE1, "keV",
            "Start of range for the energy gate for the first gamma"});
    parD.emplace("MAX_E1",
                 Conf::Property<double&>{
                     maxE1, "keV",
                     "End of range for the energy gate for the first gamma"});
    parD.emplace(
        "MIN_E2",
        Conf::Property<double&>{
            minE2, "keV",
            "Start of range for the energy gate for the second gamma"});
    parD.emplace("MAX_E2",
                 Conf::Property<double&>{
                     maxE2, "keV",
                     "End of range for the energy gate for the second gamma"});
    parD.emplace("MIN_T", Conf::Property<double&>{
                              minT, "s", "Start of range for the time gate"});
    parD.emplace("MAX_T", Conf::Property<double&>{
                              maxT, "s", "End of range for the time gate"});
    parB.emplace("SPECTRA_CRYS",
                 Conf::Property<bool&>{crysSpectra, "#",
                                       "Print spectra for every crystal"});
}
