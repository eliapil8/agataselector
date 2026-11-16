#include "SpiderConf.h"

SpiderConf::SpiderConf() : DetectorConf("SPIDER_CONF") {
    lutFile = "LUT_SPIDER.dat";
    parI.emplace("E_BINS",
                 Conf::Property<int&>{Ebins, "#",
                                         "Number of bins for spider E histograms"});
    parD.emplace("E_MAX",
                 Conf::Property<double&>{Emax, "MeV",
                                         "SPIDER histograms max energy range"});
    parD.emplace("E_MIN",
                 Conf::Property<double&>{Emin, "MeV",
                                         "SPIDER histograms max energy range"});

    parI.emplace(
        "EX_BINS",
        Conf::Property<int&>{Exbins, "#",
                                "Number of bins for spider Ex histograms"});
    parD.emplace("EX_MAX",
                 Conf::Property<double&>{Exmax, "MeV",
                                         "SPIDER histograms Ex max range"});
    parD.emplace("EX_MIN",
                 Conf::Property<double&>{Exmin, "MeV",
                                         "SPIDER histograms Ex min range"});

    parD.emplace("E_SHIFT",
                 Conf::Property<double&>{eShift, "#",
                                         "Shift in energy (recalibration)"});
    parD.emplace("E_GAIN", Conf::Property<double&>{
                               eGain, "#", "Gain in energy (recalibration)"});

    parD.emplace("EX_SHIFT",
                 Conf::Property<double&>{exShift, "#",
                                         "Shift in excitation energy (recalibration)"});
    parD.emplace("EX_GAIN", Conf::Property<double&>{
                               exGain, "#", "Gain in excitation energy (recalibration)"});
    parD.emplace("EX_QUAD", Conf::Property<double&>{
                               exQuad, "#", "Quad term in excitation energy (recalibration)"});
    parD.emplace("COS_AMPLI", Conf::Property<double&>{
                               cosAmpli, "keV", "# Amplitude of cos-like correction in Ex of SPIDER"});
    parD.emplace("COS_PHASE", Conf::Property<double&>{
                               cosPhase, "#", "# Phase of cos-like correction in Ex of SPIDER"});
    parD.emplace("COS_PERIOD", Conf::Property<double&>{
                               cosPeriod, "deg", "# Period (x/COS_PERIOD) of cos-like correction in Ex of SPIDER"});
    
    parD.emplace("DEADLAYER_THICKNESS",
                 Conf::Property<double&>{
                     deadlayer_thickness, "mg/cm2",
                     "Dead layer thickness-density in mg/cm2 or units alike"});
    parS.emplace("DEADLAYER_MATERIAL",
                 Conf::Property<std::string&>{deadlayer_material, "#",
                                              "Target material"});

    parION.emplace("ION", Conf::Property<ReactionFragment::Ion&>{
                              ion, "#", "Ion assumed in spider"});
    parB.emplace("KEEP_HIGH_ENE_EVT",
                 Conf::Property<bool&>{
                     keepHighEneEvent, "#",
                     "Only analyses the high energy events in spider for DC"});

    parMI.emplace("NEW_ID", Conf::Property<std::unordered_map<int, int>&>{
      aliasId, "#",
	"Rename a channel, only for visualization purposes"
	});
    
    parVPI.emplace("BROKEN_CHANNEL",
                   Conf::Property<std::vector<std::pair<int, int>>&>{
                       broken_channels, "#", "Broken board channel"});

    parS.emplace("E_THETA_CUT_PATH",
		 Conf::Property<std::string&>{ethetacutPath, "#", "Path with cut in E vs Theta"});

    parS.emplace("EX_THETA_PATH",
		 Conf::Property<std::string&>{exthetaPath, "#", "Path with TGraph in Ex vs Theta"});

    parS.emplace("EX_PHI_PATH",
		 Conf::Property<std::string&>{exphiPath, "#", "Path with TGraph in Ex vs Phi"});

    parS.emplace("EX_ID_PATH",
		 Conf::Property<std::string&>{exidPath, "#", "Path with TGraph in Ex vs Theta"});


}
