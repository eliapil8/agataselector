#include "OscarConf.h"

OscarConf::OscarConf(const std::string& mark) : DetectorConf(mark) {
    lutFile = "LUT_OSCAR.dat";
    parB.emplace(
        "USE_E_LAYER_ONLY",
        Conf::Property<bool&>{useElayerOnly, "#",
                              "Only use E layer for the analysis (YES/NO)"});
    parB.emplace("DE_E_HISTOS",
                 Conf::Property<bool&>{dE_E_Histos, "#",
                                       "Create matrices dE vs E for each "
                                       "pad/strip combination (YES/NO)"});
    parB.emplace(
        "GLOBAL_DE_E_CUTS",
        Conf::Property<bool&>{
            global_dE_E_Cuts, "#",
            "Apply DE vs E cuts for each pad/strip combination (YES/NO)"});
    parB.emplace("REJECT_OUTSIDE_DE_E",
                 Conf::Property<bool&>{rejectOutsideDEEnergy, "#",
                                       "Reject OSCAR events outside all E-dE cuts (YES/NO)"});
    parB.emplace("REJECT_OUTSIDE_E_THETA",
                 Conf::Property<bool&>{rejectOutsideEnergyTheta, "#",
                                       "Reject OSCAR events outside all E-theta cuts (YES/NO)"});
    parB.emplace(
        "EK_FROM_BETA_AVG",
        Conf::Property<bool&>{EkFromBetaAvg, "#",
                              "Take kinetic energy from beta average, while "
                              "direction from detector (YES/NO)"});
    parD.emplace("OSCAR_BIN_WIDTH",
                 Conf::Property<double&>{oscarbinWidth, "#",
                                         "Bin width for oscar histograms"});
    parD.emplace("OSCAR_E_MAX",
                 Conf::Property<double&>{oscarEmax, "MeV",
                                         "OSCAR histograms max range"});
    parD.emplace("OSCAR_EX_BIN_WIDTH",
                 Conf::Property<double&>{oscarExbinWidth, "#",
                                         "Bin width for oscar Ex histograms"});
    parD.emplace("OSCAR_EX_MAX",
                 Conf::Property<double&>{oscarExmax, "MeV",
                                         "OSCAR histograms Ex max range"});

    parD.emplace("E_SHIFT",
                 Conf::Property<double&>{eShift, "#",
                                         "Shift in energy (recalibration)"});
    parD.emplace("E_GAIN", Conf::Property<double&>{
                               eGain, "#", "Gain in energy (recalibration)"});
    parD.emplace("DEADLAYER_THICKNESS",
                 Conf::Property<double&>{
                     deadlayerThickness, "mg/cm2",
                     "Dead layer thickness-density in mg/cm2 or units alike"});
    parS.emplace("DEADLAYER_MATERIAL",
                 Conf::Property<std::string&>{deadlayerMaterial, "#",
                                              "Dead layer material"});
    parB.emplace("EDE_KINEMATIC_LINES",
                 Conf::Property<bool&>{edeKinematicLines, "#",
                                       "Generate theoretical E-dE kinematic lines (YES/NO)"});
    parS.emplace("EDE_REACTION_FILE",
                 Conf::Property<std::string&>{edeReactionFile, "#",
                                              "OSCAR E-dE reaction definition file"});
    parS.emplace("EDE_ACTIVE_MATERIAL",
                 Conf::Property<std::string&>{edeActiveMaterial, "#",
                                              "Material of the active OSCAR detector layers"});
    parS.emplace("E_THETA_CUT_PATH",
                 Conf::Property<std::string&>{energyThetaCutPath, "#",
                                              "Path containing OSCAR E-theta TCutG files"});
    parVP.emplace("EX_GATE",
                  Conf::Property<std::vector<std::pair<double, double>>&>{
                      excitationEnergyGates, "MeV",
                      "Accepted OSCAR excitation-energy interval; may be repeated"});

    parION.emplace("ION", Conf::Property<ReactionFragment::Ion&>{
                              ion, "#", "Ion assumed in Oscar"});
    parVPI.emplace("BROKEN_CHANNEL",
                   Conf::Property<std::vector<std::pair<int, int>>&>{
                       brokenChannels, "#", "Broken board channel"});
}

bool OscarConf::passesExcitationEnergyGate(double excitationEnergy) const
{
    if (excitationEnergyGates.empty())
        return true;
    return std::any_of(excitationEnergyGates.begin(), excitationEnergyGates.end(),
                       [excitationEnergy](const auto &gate) {
                           return excitationEnergy > gate.first && excitationEnergy < gate.second;
                       });
}

bool OscarConf::passesEnergyThetaCut(double thetaLab, double particleEnergy) const
{
    return std::any_of(energyThetaCuts.begin(), energyThetaCuts.end(),
                       [thetaLab, particleEnergy](const TCutG *cut) {
                           return cut && cut->IsInside(thetaLab / UNITS::deg,
                                                       particleEnergy / UNITS::MeV);
                       });
}
