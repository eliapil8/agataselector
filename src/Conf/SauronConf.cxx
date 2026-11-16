#include "SauronConf.h"

SauronConf::SauronConf(const std::string& mark) : DetectorConf(mark) {
    lutFile = "LUT_SAURON.dat";
    parD.emplace("SAURON_BIN_WIDTH",
                 Conf::Property<double&>{sauronbinWidth, "#",
                                         "Bin width for Sauron histograms"});
    parD.emplace("SAURON_E_MAX",
                 Conf::Property<double&>{sauronEmax, "MeV",
                                         "SAURON histograms max range"});

    parB.emplace(
        "P_SIDE_PRIORITY",
        Conf::Property<bool&>{pSidePriority, "#",
                              "P side priority for energy and time YES/NO"});

    parB.emplace(
        "SINGLE_SIDE_DETECTOR",
        Conf::Property<bool&>{singleSideDetector, "#",
                              "Disable P-N coincidences and use only one side "
                              "(the one with priority) YES/NO"});

    parB.emplace("PID_HISTOGRAMS",
                 Conf::Property<bool&>{pidHistograms, "#",
                                       "Draw pid matrix histograms"});

    parB.emplace("PID_FROMPSIDE",
                 Conf::Property<bool&>{pidFromPside, "#",
                                       "PID is taken from the pSide"});

    parB.emplace("PID_MATCH", Conf::Property<bool&>{
                                  pidMatch, "#",
                                  "Matches PID from front and back side"});

    parD.emplace("TARGET_DISTANCE",
                 Conf::Property<double&>{distanceFromTarget, "mm",
                                         "Distance from target"});

    parD.emplace(
        "SAURON_EX_BIN_WIDTH",
        Conf::Property<double&>{sauronExbinWidth, "#",
                                "Bin width for Sauron Ex histograms"});
    parD.emplace("SAURON_EX_MAX",
                 Conf::Property<double&>{sauronExmax, "MeV",
                                         "Sauron histograms Ex max range"});

    parD.emplace("E_SHIFT",
                 Conf::Property<double&>{eShift, "#",
                                         "Shift in energy (recalibration) "});
    parD.emplace("E_GAIN", Conf::Property<double&>{
                               eGain, "#", "Gain in energy (recalibration) "});

    parD.emplace(
        "ENE_WINDOW_LEFT",
        Conf::Property<double&>{eneWindowLeft, "#",
                                "Tolerance for energy P-N coincidence"});
    parD.emplace(
        "ENE_WINDOW_RIGHT",
        Conf::Property<double&>{eneWindowRight, "#",
                                "Tolerance for energy P-N coincidence"});

    parD.emplace("DEADLAYER_THICKNESS",
                 Conf::Property<double&>{
                     deadlayerThickness, "mg/cm2",
                     "Dead layer thickness-density in mg/cm2 or units alike"});
    parS.emplace("DEADLAYER_MATERIAL",
                 Conf::Property<std::string&>{deadlayerMaterial, "#",
                                              "Dead layer material"});
    parD.emplace("ABSORBER_THICKNESS",
                 Conf::Property<double&>{
                     absorberThickness, "mg/cm2",
                     "Absorber thickness-density in mg/cm2 or units alike"});
    parS.emplace("ABSORBER_MATERIAL",
                 Conf::Property<std::string&>{absorberMaterial, "#",
                                              "Absorber material"});
    parION.emplace("ION", Conf::Property<ReactionFragment::Ion&>{
                              ion, "#", "Ion assumed in Sauron"});

    parVPI.emplace("BROKEN_CHANNEL",
                   Conf::Property<std::vector<std::pair<int, int>>&>{
                       brokenChannels, "#", "Broken board channel"});
}
