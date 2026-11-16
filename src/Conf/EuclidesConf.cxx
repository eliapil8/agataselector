#include "EuclidesConf.h"

EuclidesConf::EuclidesConf() : DetectorConf("EUCLIDES_CONF") {
    lutFile = "LUT_EUCLIDES.dat";

    parD.emplace("E_SHIFT",
                 Conf::Property<double&>{eShift, "#",
                                         "Shift in energy (recalibration)"});
    parD.emplace("E_GAIN", Conf::Property<double&>{
                               eGain, "#", "Gain in energy (recalibration)"});
}
