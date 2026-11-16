#include "DetectorConf.h"

DetectorConf::DetectorConf(const std::string& mark) : Conf(mark) {
    parI.emplace(
        "COINC_W_LEFT",
        Conf::Property<int&>{windowLeft, "#",
                             "Time window left with the same type of det"});
    parI.emplace("COINC_W_RIGHT",
                 Conf::Property<int&>{
                     windowRight, "#",
                     "Time window right with with the same type of det"});
    parI.emplace("COINC_W_LEFT_LEFT_BKG",
                 Conf::Property<int&>{windowLeftLeftBkg, "#",
                                      "Left Background time window on the "
                                      "left side of the coincidence peak"});
    parI.emplace("COINC_W_RIGHT_LEFT_BKG",
                 Conf::Property<int&>{windowRightLeftBkg, "#",
                                      "Right Background time window on the "
                                      "left side of the coincidence peak"});
    parI.emplace("COINC_W_LEFT_RIGHT_BKG",
                 Conf::Property<int&>{windowLeftRightBkg, "#",
                                      "Left Background time window on the "
                                      "right side of the coincidence peak"});
    parI.emplace("COINC_W_RIGHT_RIGHT_BKG",
                 Conf::Property<int&>{windowRightRightBkg, "#",
                                      "Right Background time window on the "
                                      "right side of the coincidence peak"});
    parD.emplace("ANGLE",
                 Conf::Property<double&>{
                     angle, "deg",
                     "Detector angle. For AGATA should be 180-prisma_angle"});
    parD.emplace("X_SHIFT", Conf::Property<double&>{
                                xShift, "mm",
                                "Detector x shift for optimization purposes"});
    parD.emplace("Y_SHIFT", Conf::Property<double&>{
                                yShift, "mm",
                                "Detector y shift for optimization purposes"});
    parD.emplace("Z_SHIFT", Conf::Property<double&>{
                                zShift, "mm",
                                "Detector z shift for optimization purposes"});
    parD.emplace(
        "PHI",
        Conf::Property<double&>{
            phi, "deg", "Detector phi rotation for optimization purposes"});
    parD.emplace("THETA_X",
                 Conf::Property<double&>{thetaX, "deg",
                                         "Detector rotation on the horizontal "
                                         "plane for optimization purposes"});
    parD.emplace("THETA_Y",
                 Conf::Property<double&>{thetaY, "deg",
                                         "Detector rotation on the vertical "
                                         "plane for optimization purposes"});
    parD.emplace("TIME_UNIT",
                 Conf::Property<double&>{time_unit, "ns",
                                         "Timestamp unit, should 10*ns"});
    parD.emplace("CFD_UNIT",
                 Conf::Property<double&>{cfd_unit, "ns", "Cfd units"});
    parD.emplace(
        "BETA_AVG_MAG",
        Conf::Property<double&>{
            betaAvgMag, "#",
            "Average beta magnitude Doppler correction for detector"});
    parD.emplace("BETA_AVG_THETA",
                 Conf::Property<double&>{
                     betaAvgTheta, "deg",
                     "Average beta theta Doppler correction for detector"});
    parD.emplace("BETA_AVG_PHI",
                 Conf::Property<double&>{
                     betaAvgPhi, "deg",
                     "Average beta phi Doppler correction for detector"});
    parB.emplace("ENABLE_HISTS",
                 Conf::Property<bool&>{
                     enable_histos, "#",
                     "Ebable or disable detector histos to save memory"});
    parB.emplace("ENABLE_TREE",
                 Conf::Property<bool&>{enable_treeFill, "#",
                                       "Ebable or disable detector TTree to "
                                       "save memory (strongly encouraged)"});
    parS.emplace("LUT", Conf::Property<std::string&>{lutFile, "#",
                                                     "Lookup table path"});
}
