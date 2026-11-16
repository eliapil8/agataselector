#include "LabrConf.h"

LabrConf::LabrConf() : GammadetectorConf("LABR_CONF") {
    cfd_unit    = 1 * UNITS::picosecond;
    lutFile     = "LUT_LABR.dat";
    spectraType = CORE;

    parD.emplace("E_SHIFT",
                 Conf::Property<double&>{eShift, "#",
                                         "Shift in energy (recalibratyion)"});
    parD.emplace("E_GAIN", Conf::Property<double&>{
                               eGain, "#", "Gain in energy (recalibratyion)"});
    parI.emplace("REFERENCE", Conf::Property<int&>{
                               reference, "#", "Reference ID for time alignment"});
    parION.emplace("ION", Conf::Property<ReactionFragment::Ion&>{
                              ion, "#", "Ion assumed in labr"});
}
