#include "MonitorConf.h"

MonitorConf::MonitorConf() : DetectorConf("MONITOR_CONF") {
    lutFile = "LUT_MONITOR.dat";
    parD.emplace("MONITOR_BIN_WIDTH",
                 Conf::Property<double&>{monitorbinWidth, "#",
                                         "Bin width for monitor histograms"});
    parD.emplace("MONITOR_E_MAX",
                 Conf::Property<double&>{monitorEmax, "MeV",
                                         "MONITOR histograms max range"});
    parD.emplace("E_SHIFT",
                 Conf::Property<double&>{eShift, "#",
                                         "Shift in energy (recalibration)"});
    parD.emplace("E_GAIN", Conf::Property<double&>{
                               eGain, "#", "Gain in energy (recalibration)"});
    parD.emplace("DEADLAYER_THICKNESS",
                 Conf::Property<double&>{
                     deadlayer_thickness, "mg/cm2",
                     "Dead layer thickness-density in mg/cm2 or units alike"});
    parS.emplace("DEADLAYER_MATERIAL",
                 Conf::Property<std::string&>{deadlayer_material, "#",
                                              "Target material"});

    parION.emplace("ION", Conf::Property<ReactionFragment::Ion&>{
                              ion, "#", "Ion assumed in monitor"});

}
