#include "EXSSiDeConf.h"

EXSSiDeConf::EXSSiDeConf() : DetectorConf("EXSSIDE_CONF") {
    lutFile = "LUT_EXSSiDe.dat";
    parD.emplace("BIN_WIDTH",
                 Conf::Property<double&>{exssidebinWidth, "#",
                                         "Bin width for EXSSIDE histograms"});
    parD.emplace("E_MAX",
                 Conf::Property<double&>{exssideEmax, "MeV",
                                         "EXSSIDE histograms max range"});

    parB.emplace(
        "P_SIDE_PRIORITY",
        Conf::Property<bool&>{pSidePriority, "#",
                              "P side priority for energy and time YES/NO"});

    parB.emplace(
        "SINGLE_SIDE_DETECTOR",
        Conf::Property<bool&>{singleSideDetector, "#",
                              "Disable P-N coincidences and use only one side "
                              "(the one with priority) YES/NO"});
    parD.emplace("TARGET_DISTANCE",
                 Conf::Property<double&>{distanceFromTarget, "mm",
                                         "Distance from target"});

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
                              ion, "#", "Ion assumed in EXSSIDE"});

    parVPI.emplace("BROKEN_CHANNEL",
                   Conf::Property<std::vector<std::pair<int, int>>&>{
                       brokenChannels, "#", "Broken board channel"});
    
    parUI.emplace("AVG_X",
                 Conf::Property<unsigned int&>{
                     avgx, "#",
                     "Average X of the ions [digi channel]"});    
    parUI.emplace("AVG_Y",
                 Conf::Property<unsigned int&>{
                     avgy, "#",
                     "Average Y of the ions [digi channel]"});  
    parUI.emplace("REFERENCE_CHANNEL",
                 Conf::Property<unsigned int&>{
                     refch, "#",
                     "Reference channel for Time coincidences [digi channel]"}); 
    parD.emplace("MIN_E",
                 Conf::Property<double&>{
                     minE, "#",
                     "Minimum E value above which events are processed in the analysis"});   
                                        
}
