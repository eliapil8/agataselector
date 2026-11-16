#include "GammadetectorOscarAnalysis.h"

GammadetectorOscarAnalysis::GammadetectorOscarAnalysis(Inputs                       &in,
                                                       GammadetectorOscarConf       &gpConf,
                                                       Gammadetector                &g,
                                                       GammadetectorConf            &gConf,
                                                       Oscar                        &s,
                                                       OscarConf                    &sc,
                                                       Readers                      &rg,
                                                       OscarAnalysis::OscarReader   &ro,
                                                       std::vector<GammaDC>         &gdc,
                                                       GammadetectorOscarHistograms &gsh)
    : CoincidenceAnalysis(in, g, gConf, s, sc, gpConf, rg, ro, gsh), gammadetector(g), oscar(s),
      oscarConf(sc), gammadetectorOscarConf(gpConf), gammadetectorConf(gConf), gammaDC(gdc),
      gammadetectorOscarHistograms(gsh), oscarReader(ro)
{

    (void)oscarReader;
}

void GammadetectorOscarAnalysis::AllocateEnergyThetaHistograms(
    GammadetectorOscarHistograms::EThetaSelection &selection)
{
    const int gammaBins = static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                           gammadetectorConf.binWidth);

    Initialize<TH1F>(
        selection.h_Ex,
        new TH1F("h_Ex", "OSCAR excitation energy;Excitation Energy [MeV];Counts",
                 static_cast<int>(oscarConf.oscarExmax / UNITS::MeV /
                                  oscarConf.oscarExbinWidth) +
                     static_cast<int>(10 / oscarConf.oscarExbinWidth),
                 -10, oscarConf.oscarExmax / UNITS::MeV),
        selection.ptrs);
    Initialize<TH2F>(
        selection.m_EdE,
        new TH2F("m_EdE", "OSCAR residual E vs dE;Residual E [MeV];Delta E [MeV]", 1600,
                 0, 80, 1600, 0, 80),
        selection.ptrs);
    Initialize<TH2F>(
        selection.m_Etot_dE,
        new TH2F("m_Etot_dE",
                 "OSCAR corrected total particle energy vs dE;Corrected particle energy "
                 "[MeV];Delta E [MeV]",
                 1600, 0, 80, 1600, 0, 80),
        selection.ptrs);

    Initialize<TH1D>(selection.h_E,
                     new TH1D("h_E", "Gamma energy; Energy [keV]; Counts/keV", gammaBins, 0,
                              gammadetectorConf.gammaEmax / UNITS::keV),
                     selection.ptrs);
    Initialize<TH1D>(selection.h_EDC,
                     new TH1D("h_EDC", "Doppler-corrected energy; Energy [keV]; Counts/keV",
                              gammaBins, 0, gammadetectorConf.gammaEmax / UNITS::keV),
                     selection.ptrs);
    Initialize<TH1D>(selection.h_EDCBP,
                     new TH1D("h_EDCBP",
                              "Doppler-corrected energy for binary partner; Energy [keV]; "
                              "Counts/keV",
                              gammaBins, 0, gammadetectorConf.gammaEmax / UNITS::keV),
                     selection.ptrs);

    Initialize<TH2D>(selection.m_E_Theta,
                     new TH2D("m_E_Theta", "Gamma-theta; Theta; Energy [keV]", 1000, 0, 180,
                              gammaBins, 0, gammadetectorConf.gammaEmax / UNITS::keV),
                     selection.ptrs);
    Initialize<TH2D>(selection.m_E_ThetaBP,
                     new TH2D("m_E_ThetaBP", "Gamma-theta for binary partner; Theta; Energy "
                                                  "[keV]",
                              1000, 0, 180, gammaBins, 0,
                              gammadetectorConf.gammaEmax / UNITS::keV),
                     selection.ptrs);
    Initialize<TH2D>(selection.m_EDCBP_Theta,
                     new TH2D("m_EDCBP_Theta", "DCBP energy-theta; Theta; Energy [keV]", 1000,
                              0, 180, gammaBins, 0,
                              gammadetectorConf.gammaEmax / UNITS::keV),
                     selection.ptrs);
    Initialize<TH2D>(selection.m_EDCBP_ThetaBP,
                     new TH2D("m_EDCBP_ThetaBP",
                              "DCBP energy-theta for binary partner; Theta; Energy [keV]", 1000,
                              0, 180, gammaBins, 0,
                              gammadetectorConf.gammaEmax / UNITS::keV),
                     selection.ptrs);

    Initialize<TH2D>(selection.m_E_E,
                     new TH2D("m_E_E", "Gamma-gamma energy;Energy [keV];Energy [keV]",
                              gammaBins, 0, gammadetectorConf.gammaEmax / UNITS::keV, gammaBins, 0,
                              gammadetectorConf.gammaEmax / UNITS::keV),
                     selection.ptrs);
    Initialize<TH2D>(selection.m_EDC_EDC,
                     new TH2D("m_EDC_EDC",
                              "Doppler corrected gamma-gamma energy;Energy [keV];Energy [keV]",
                              gammaBins, 0, gammadetectorConf.gammaEmax / UNITS::keV, gammaBins, 0,
                              gammadetectorConf.gammaEmax / UNITS::keV),
                     selection.ptrs);
    Initialize<TH2D>(selection.m_EDCBP_EDCBP,
                     new TH2D("m_EDCBP_EDCBP",
                              "Doppler corrected gamma-gamma energy for binary partner;Energy "
                              "[keV];Energy [keV]",
                              gammaBins, 0, gammadetectorConf.gammaEmax / UNITS::keV, gammaBins, 0,
                              gammadetectorConf.gammaEmax / UNITS::keV),
                     selection.ptrs);
    Initialize<TH2D>(selection.m_E_EDC,
                     new TH2D("m_E_EDC",
                              "Gamma energy vs Doppler corrected gamma energy;Energy "
                              "[keV];Energy DC[keV]",
                              gammaBins, 0, gammadetectorConf.gammaEmax / UNITS::keV, gammaBins, 0,
                              gammadetectorConf.gammaEmax / UNITS::keV),
                     selection.ptrs);
    Initialize<TH2D>(selection.m_E_EDCBP,
                     new TH2D("m_E_EDCBP",
                              "Gamma energy vs Doppler corrected energy for binary "
                              "partner;Energy [keV];Energy DCBP[keV]",
                              gammaBins, 0, gammadetectorConf.gammaEmax / UNITS::keV, gammaBins, 0,
                              gammadetectorConf.gammaEmax / UNITS::keV),
                     selection.ptrs);
    Initialize<TH2D>(selection.m_EDC_EDCBP,
                     new TH2D("m_EDC_EDCBP",
                              "Doppler corrected energy vs energy corrected for binary "
                              "partner;Energy DC[keV];Energy DCBP[keV]",
                              gammaBins, 0, gammadetectorConf.gammaEmax / UNITS::keV, gammaBins, 0,
                              gammadetectorConf.gammaEmax / UNITS::keV),
                     selection.ptrs);
    Initialize<TH2D>(selection.m_E_Beta,
                     new TH2D("m_E_Beta", "Gamma - Beta;Beta [%];Energy [keV]", 4000, 0, 20,
                              gammaBins, 0, gammadetectorConf.gammaEmax / UNITS::keV),
                     selection.ptrs);
    Initialize<TH2D>(selection.m_E_BetaBP,
                     new TH2D("m_E_BetaBP", "Gamma - Beta BP;Beta [%];Energy [keV]", 4000, 0,
                              20, gammaBins, 0, gammadetectorConf.gammaEmax / UNITS::keV),
                     selection.ptrs);
}

bool GammadetectorOscarAnalysis::AllocateHistos()
{
    GammadetectorOscarHistograms &h = gammadetectorOscarHistograms;
    h.dir->cd();

    if (!CoincidenceAnalysis::AllocateHistos())
        return false;

    gammadetectorOscarConf.betaAvg.SetMagThetaPhi(gammadetectorOscarConf.betaAvgMag,
                                                  gammadetectorOscarConf.betaAvgTheta,
                                                  gammadetectorOscarConf.betaAvgPhi);

    // Initialize<TH2D>(h.m_TSdiff_TS,
    //                  new TH2D("m_TSdiff_TS", "m_TSdiff_TS", 12 * 360, 0,
    //                           12 * 3600,
    //                           (gammadetectorOscarConf.windowRight
    //                            - gammadetectorOscarConf.windowLeft + 200),
    //                           gammadetectorOscarConf.windowLeft - 100,
    //                           gammadetectorOscarConf.windowRight + 100),
    //                  h.ptrs);

    // Initialize<TH2D>(
    //     h.m_TSoscarmTSgammadetector_oscarID,
    //     new TH2D(
    //         "m_TSoscarmTSgammadetector_oscarID",
    //         "OscarTS - GammadetectorTS vs OSCAR segment; OSCAR segment;TS "
    //         "difference [10 ns]",
    //         90, 0, 90,
    //         (gammadetectorOscarConf.windowRight
    //          - gammadetectorOscarConf.windowLeft + 200)
    //             * 10,
    //         gammadetectorOscarConf.windowLeft - 100,
    //         oscarConf.windowRight + 100),
    //     h.ptrs);
    // SetupLabelsForChannelID(oscarConf, h.m_TSoscarmTSgammadetector_oscarID);

    Initialize<TH1D>(
        h.h_cTime_gated,
        new TH1D("h_cTime_gated", "h_cTime_gated",
                 (gammadetectorOscarConf.windowRight - gammadetectorOscarConf.windowLeft + 200) *
                     10,
                 gammadetectorOscarConf.windowLeft - 100, gammadetectorOscarConf.windowRight + 100),
        h.ptrs);

    Initialize<TH1D>(
        h.h_E,
        new TH1D(
            "h_E", Form("Gamma energy;Energy [keV];Counts/%d keV", gammadetectorConf.binWidth),
            static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV / gammadetectorConf.binWidth),
            0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH1D>(h.h_EDC,
                     new TH1D("h_EDC",
                              Form("Doppler corrected gamma energy;Energy [keV];Counts/%d keV",
                                   gammadetectorConf.binWidth),
                              static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                               gammadetectorConf.binWidth),
                              0, gammadetectorConf.gammaEmax / UNITS::keV),
                     h.ptrs);

    Initialize<TH1D>(h.h_EDCBP,
                     new TH1D("h_EDCBP",
                              Form("Doppler corrected gamma energy for binary "
                                   "partner;Energy [keV];Counts/%d keV",
                                   gammadetectorConf.binWidth),
                              static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                               gammadetectorConf.binWidth),
                              0, gammadetectorConf.gammaEmax / UNITS::keV),
                     h.ptrs);

    Initialize<TH1D>(h.h_EDC_avg,
                     new TH1D("h_EDC_avg",
                              Form("Histogram with average Doppler correction;Energy "
                                   "[keV];Counts/%d keV",
                                   gammadetectorConf.binWidth),
                              static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                               gammadetectorConf.binWidth),
                              0, gammadetectorConf.gammaEmax / UNITS::keV),
                     h.ptrs);

    Initialize<TH2D>(
        h.m_E_E,
        new TH2D(
            "m_E_E", "Gamma-gamma energy;Energy [keV];Energy [keV]",
            static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV / gammadetectorConf.binWidth),
            0, gammadetectorConf.gammaEmax / UNITS::keV,
            static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV / gammadetectorConf.binWidth),
            0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_EDC_EDC,
        new TH2D(
            "m_EDC_EDC", "Doppler corrected gamma-gamma energy;Energy [keV];Energy [keV]",
            static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV / gammadetectorConf.binWidth),
            0, gammadetectorConf.gammaEmax / UNITS::keV,
            static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV / gammadetectorConf.binWidth),
            0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_EDCBP_EDCBP,
        new TH2D(
            "m_EDCBP_EDCBP",
            "Doppler corrected gamma-gamma energy for binary "
            "partner;Energy [keV];Energy [keV]",
            static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV / gammadetectorConf.binWidth),
            0, gammadetectorConf.gammaEmax / UNITS::keV,
            static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV / gammadetectorConf.binWidth),
            0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_E_EDC,
        new TH2D(
            "m_E_EDC",
            "Gamma energy vs Doppler corrected gamma energy;Energy "
            "[keV];Energy DC[keV]",
            static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV / gammadetectorConf.binWidth),
            0, gammadetectorConf.gammaEmax / UNITS::keV,
            static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV / gammadetectorConf.binWidth),
            0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_E_EDCBP,
        new TH2D(
            "m_E_EDCBP",
            "Gamma energy vs Doppler corrected energy for binary "
            "partner;Energy [keV];Energy DCBP[keV]",
            static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV / gammadetectorConf.binWidth),
            0, gammadetectorConf.gammaEmax / UNITS::keV,
            static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV / gammadetectorConf.binWidth),
            0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_EDC_EDCBP,
        new TH2D(
            "m_EDC_EDCBP",
            "Doppler corrected energy vs energy doppler corrected for "
            "binary partner;Energy DC[keV];Energy DCBP[keV]",
            static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV / gammadetectorConf.binWidth),
            0, gammadetectorConf.gammaEmax / UNITS::keV,
            static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV / gammadetectorConf.binWidth),
            0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(h.m_E_Beta,
                     new TH2D("m_E_Beta", "Gamma - Beta;Beta [%];Energy [keV]", 4000, 0, 20,
                              static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                               gammadetectorConf.binWidth),
                              0, gammadetectorConf.gammaEmax / UNITS::keV),
                     h.ptrs);

    Initialize<TH2D>(h.m_E_BetaBP,
                     new TH2D("m_E_BetaBP", "Gamma - Beta BP;Beta [%];Energy [keV]", 4000, 0, 20,
                              static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                               gammadetectorConf.binWidth),
                              0, gammadetectorConf.gammaEmax / UNITS::keV),
                     h.ptrs);

    Initialize<TH2D>(
        h.m_EDC_Beta,
        new TH2D(
            "m_EDC_Beta", "Gamma Doppler corrected - Beta;Beta [%];Energy [keV]", 4000, 0, 20,
            static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV / gammadetectorConf.binWidth),
            0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(h.m_EDCBP_BetaBP,
                     new TH2D("m_EDCBP_BetaBP",
                              "Gamma Doppler corrected for binary partner - Beta BP;Beta "
                              "[%];Energy [keV]",
                              4000, 0, 20,
                              static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                               gammadetectorConf.binWidth),
                              0, gammadetectorConf.gammaEmax / UNITS::keV),
                     h.ptrs);

    Initialize<TH2D>(
        h.m_E_Theta,
        new TH2D(
            "m_E_Theta", "Gamma - theta energy;Theta [deg];Energy [keV]", 1000, 0, 180,
            static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV / gammadetectorConf.binWidth),
            0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_E_ThetaBP,
        new TH2D(
            "m_E_ThetaBP", "Gamma - theta for binary partner;Theta [deg];Energy [keV]", 1000, 0,
            180,
            static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV / gammadetectorConf.binWidth),
            0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_EDCBP_ThetaBP,
        new TH2D(
            "m_EDCBP_ThetaBP", "Gamma - theta energy for binary partner;Theta [deg];Energy [keV]",
            1000, 0, 180,
            static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV / gammadetectorConf.binWidth),
            0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_EDC_Theta,
        new TH2D(
            "m_EDC_Theta", "Doppler corrected gamma - theta;Angle [deg];Energy [keV]", 1000, 0, 180,
            static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV / gammadetectorConf.binWidth),
            0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(h.m_EDCBP_Theta,
                     new TH2D("m_EDCBP_Theta",
                              "Doppler corrected gamma- theta for binary partner;Angle "
                              "[deg];Energy [keV]",
                              1000, 0, 180,
                              static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                               gammadetectorConf.binWidth),
                              0, gammadetectorConf.gammaEmax / UNITS::keV),
                     h.ptrs);

    Initialize<TH2D>(h.m_EDCBP_TS,
                     new TH2D("m_EDCBP_TS", // Added by MR (be careful!)
                              "Doppler corrected gamma-TS for binary partner;TimeStamp "
                              "[s];Energy [keV]",
                              4320, 0, 12 * 3600,
                              static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                               gammadetectorConf.binWidth),
                              0, gammadetectorConf.gammaEmax / UNITS::keV),
                     h.ptrs);

    Initialize<TH2D>(
        h.m_Ex_E,
        new TH2D(
            "m_Ex_E",
            "Excitation energy - gamma energy;Excitation Energy "
            "[MeV];Gamma Energy [keV]",
            static_cast<int>(oscarConf.oscarExmax / UNITS::MeV / oscarConf.oscarExbinWidth) +
                static_cast<int>(10 / oscarConf.oscarExbinWidth),
            -10, oscarConf.oscarExmax / UNITS::MeV,
            static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV / gammadetectorConf.binWidth),
            0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_Ex_EDC,
        new TH2D(
            "m_Ex_EDC",
            "Excitation energy - Doppler corrected gamma "
            "energy;Excitation Energy [MeV];Gamma Energy [keV]",
            static_cast<int>(oscarConf.oscarExmax / UNITS::MeV / oscarConf.oscarExbinWidth) +
                static_cast<int>(10 / oscarConf.oscarExbinWidth),
            -10, oscarConf.oscarExmax / UNITS::MeV,
            static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV / gammadetectorConf.binWidth),
            0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_Ex_EDCBP,
        new TH2D(
            "m_Ex_EDCBP",
            "Excitation energy - Doppler corrected gamma energy for "
            "binary partner;Excitation Energy [MeV];Gamma Energy [keV]",
            static_cast<int>(oscarConf.oscarExmax / UNITS::MeV / oscarConf.oscarExbinWidth) +
                static_cast<int>(10 / oscarConf.oscarExbinWidth),
            -10, oscarConf.oscarExmax / UNITS::MeV,
            static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV / gammadetectorConf.binWidth),
            0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);
    Initialize<TH2D>(h.m_OscarID_EDC,
                     new TH2D("m_OscarID_EDC",
                              "OSCAR ID (strip) - Doppler corrected gamma energy"
                              ";OSCAR ID;Gamma Energy [keV]",
                              96, 0, 96,
                              static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                               gammadetectorConf.binWidth) *
                                  2,
                              0, gammadetectorConf.gammaEmax / UNITS::keV),
                     h.ptrs);
    SetupLabelsForChannelID(oscarConf, h.m_OscarID_EDC);
    Initialize<TH2D>(h.m_OscarID_EDCBP,
                     new TH2D("m_OscarID_EDCBP",
                              "OSCAR ID (strip) - Doppler corrected gamma energy for "
                              "binary partner;OSCAR ID;Gamma Energy [keV]",
                              96, 0, 96,
                              static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                               gammadetectorConf.binWidth) *
                                  2,
                              0, gammadetectorConf.gammaEmax / UNITS::keV),
                     h.ptrs);
    SetupLabelsForChannelID(oscarConf, h.m_OscarID_EDCBP);

    // Histograms with condition on Excitation energy
    for (unsigned int i{0}; i < gammadetectorOscarConf.exValues.size(); ++i)
    {
        h.exSelection.emplace_back();
        auto &vals = gammadetectorOscarConf.exValues.at(i);

        h.exSelection.back().vals = vals;
        h.exSelection.back().dir =
            h.dir->mkdir(Form("ExGate_Ei%.3f_Ef%.3f", vals.first, vals.second),
                         Form("Ex gated histograms, Ei=%.3f Ef=%.3f", vals.first, vals.second));
        h.exSelection.back().dir->cd();

        Initialize<TH1D>(h.exSelection.back().h_E,
                         new TH1D("h_E", "Gamma energy; Energy [keV]; Counts/keV",
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV),
                         h.exSelection.back().ptrs);

        Initialize<TH1D>(h.exSelection.back().h_EDC,
                         new TH1D("h_EDC", " Doppler-corrected energy; Energy [keV]; Counts/keV",
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV),
                         h.exSelection.back().ptrs);

        Initialize<TH1D>(h.exSelection.back().h_EDCBP,
                         new TH1D("h_EDCBP",
                                  "Doppler-corrected energy for binary partner; Energy "
                                  "[keV]; Counts/keV",
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV),
                         h.exSelection.back().ptrs);

        Initialize<TH2D>(h.exSelection.back().m_E_Theta,
                         new TH2D("m_E_Theta", "Gamma-theta; Theta; Energy [keV]", 1000, 0, 180,
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV),
                         h.exSelection.back().ptrs);

        Initialize<TH2D>(h.exSelection.back().m_E_ThetaBP,
                         new TH2D("m_E_ThetaBP",
                                  "Gamma-theta for binary partner; Theta; Energy [keV]", 1000, 0,
                                  180,
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV),
                         h.exSelection.back().ptrs);

        Initialize<TH2D>(h.exSelection.back().m_EDCBP_Theta,
                         new TH2D("m_EDCBP_Theta", " DCBP energy-theta; Theta; Energy [keV]", 1000,
                                  0, 180,
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV),
                         h.exSelection.back().ptrs);

        Initialize<TH2D>(h.exSelection.back().m_EDCBP_ThetaBP,
                         new TH2D("m_EDCBP_ThetaBP",
                                  "DCBP energy - theta for binary partner; Theta; Energy [keV]",
                                  1000, 0, 180,
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV),
                         h.exSelection.back().ptrs);

        Initialize<TH2D>(h.exSelection.back().m_E_E,
                         new TH2D("m_E_E", "Gamma-gamma energy;Energy [keV];Energy [keV]",
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV,
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV),
                         h.exSelection.back().ptrs);

        Initialize<TH2D>(h.exSelection.back().m_EDC_EDC,
                         new TH2D("m_EDC_EDC",
                                  "Doppler corrected gamma-gamma energy;Energy "
                                  "[keV];Energy [keV]",
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV,
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV),
                         h.exSelection.back().ptrs);

        Initialize<TH2D>(h.exSelection.back().m_EDCBP_EDCBP,
                         new TH2D("m_EDCBP_EDCBP",
                                  "Doppler corrected gamma-gamma energy for binary "
                                  "partner;Energy [keV];Energy [keV]",
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV,
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV),
                         h.exSelection.back().ptrs);

        Initialize<TH2D>(h.exSelection.back().m_E_EDC,
                         new TH2D("m_E_EDC",
                                  "Gamma energy vs Doppler corrected gamma energy;Energy "
                                  "[keV];Energy DC[keV]",
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV,
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV),
                         h.exSelection.back().ptrs);

        Initialize<TH2D>(h.exSelection.back().m_E_EDCBP,
                         new TH2D("m_E_EDCBP",
                                  "Gamma energy vs Doppler corrected energy for binary "
                                  "partner;Energy [keV];Energy DCBP[keV]",
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV,
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV),
                         h.exSelection.back().ptrs);

        Initialize<TH2D>(h.exSelection.back().m_EDC_EDCBP,
                         new TH2D("m_EDC_EDCBP",
                                  "Doppler corrected energy vs energy doppler corrected "
                                  "for binary partner;Energy DC[keV];Energy DCBP[keV]",
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV,
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV),
                         h.exSelection.back().ptrs);

        Initialize<TH2D>(h.exSelection.back().m_E_Beta,
                         new TH2D("m_E_Beta", "Gamma - Beta;Beta [%];Energy [keV]", 4000, 0, 20,
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV),
                         h.exSelection.back().ptrs);

        Initialize<TH2D>(h.exSelection.back().m_E_BetaBP,
                         new TH2D("m_E_BetaBP", "Gamma - Beta;Beta [%];Energy [keV]", 4000, 0, 20,
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV),
                         h.exSelection.back().ptrs);

        // Adding TGraph with kinematic line
        // we should probably check here if this isn't duplicite in theta angle?
        auto reactionsIt = input.reactions.find(oscarConf.ion);
        if (reactionsIt == input.reactions.end())
            throw std::runtime_error("Ion in oscar not found in reactions of "
                                     "interest! Check the conf file\n");

        std::string sTemp = input.treeToRead->GetDirectory()
                                ->GetFile()
                                ->GetName(); // to avoid potential memory leak message of hadd
        sTemp.erase(sTemp.begin(), sTemp.begin() + sTemp.find(".root") - 4);
        sTemp.erase(sTemp.end() - 5, sTemp.end());

        if (sTemp == "0000")
        {
            h.exSelection.back().dir->Add(h.exSelection.back().g_Energy_Theta =
                                              GenerateKinematicLine(vals.first));
            h.exSelection.back().dir->Add(h.exSelection.back().g_Energy_Theta2 =
                                              GenerateKinematicLine(vals.second));
        }
    }

    // Match SPIDER's coincidence-level E-theta selection: every TCutG receives
    // its own output directory and the standard gated gamma-histogram set.
    conf.AddCuts<GammadetectorOscarHistograms>(oscarConf.energyThetaCutPath,
                                               h.energyThetaSelections);
    for (auto &selection : h.energyThetaSelections)
    {
        selection.dir = h.dir->mkdir(Form("E_Theta_%s", selection.cut->GetName()),
                                     "Gamma spectra with cut on OSCAR Elab - ThetaLab");
        selection.dir->cd();
        selection.cut->Write();
        AllocateEnergyThetaHistograms(selection);
    }

    for (const auto &reaction : input.reactions)
    {
        GammadetectorOscarHistograms::ReactionHistograms &reactionHistos =
            h.reactionHistos[{reaction.first}];
        std::string histoName =
            "_A" + std::to_string(reaction.first.A) + "Z" + std::to_string(reaction.first.Z);
        std::string histoTitle =
            " A=" + std::to_string(reaction.first.A) + " Z=" + std::to_string(reaction.first.Z);

        reactionHistos.dir = h.dir->mkdir(
            Form("Reaction%s", histoName.c_str()),
            Form("Gamma spectra with cuts on the identified ion: %s", histoTitle.c_str()));
        reactionHistos.dir->cd();

        //

        Initialize<TH1D>(reactionHistos.h_E,
                         new TH1D(Form("h_E_ion%s", histoName.c_str()),
                                  Form("E spectrum for %s;Energy [keV];Counts/%d keV",
                                       histoTitle.c_str(), gammadetectorConf.binWidth),
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV),
                         reactionHistos.ptrs);

        Initialize<TH1D>(reactionHistos.h_EDCBP,
                         new TH1D(Form("h_EDCBP_ion%s", histoName.c_str()),
                                  Form("DC spectrum for %s;Energy DC [keV];Counts/%d keV",
                                       histoTitle.c_str(), gammadetectorConf.binWidth),
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV),
                         reactionHistos.ptrs);

        Initialize<TH1D>(reactionHistos.h_EDC_avg,
                         new TH1D(Form("h_EDC_avg_ion%s", histoName.c_str()),
                                  Form("Histogram with average Doppler correction for ion %s;"
                                       "Energy [keV];Counts/%d keV",
                                       histoTitle.c_str(), gammadetectorConf.binWidth),
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV),
                         h.ptrs);

        Initialize<TH2D>(reactionHistos.m_Ex_E,
                         new TH2D(Form("m_Ex_E_ion%s", histoName.c_str()),
                                  Form("Excitation energy spectrum vs gamma energy for "
                                       "%s;Energy [keV];Excitation energy [MeV]",
                                       histoTitle.c_str()),
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV, 1000, -10, 30),
                         reactionHistos.ptrs);

        Initialize<TH2D>(reactionHistos.m_Ex_EDCBP,
                         new TH2D(Form("m_Ex_DCEBP_ion%s", histoName.c_str()),
                                  Form("Excitation energy spectrum vs DC gamma energy for "
                                       "%s;Energy DC [keV];Excitation energy [MeV]",
                                       histoTitle.c_str()),
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV, 1000, -10, 30),
                         reactionHistos.ptrs);

        Initialize<TH2D>(reactionHistos.m_E_E,
                         new TH2D(Form("m_E_E_ion%s", histoName.c_str()),
                                  Form("Gamma-gamma energy for %s;Energy [keV];Energy [keV]",
                                       histoTitle.c_str()),
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV,
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV),
                         reactionHistos.ptrs);

        Initialize<TH2D>(reactionHistos.m_E_ThetaBP,
                         new TH2D(Form("m_E_ThetaBP_ion%s", histoName.c_str()),
                                  Form("Gamma - theta for binary partner of ion %s;Theta [deg];"
                                       "Energy [keV]",
                                       histoTitle.c_str()),
                                  1000, 0, 180,
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV),
                         reactionHistos.ptrs);
        Initialize<TH2D>(reactionHistos.m_OscarTel_Ex,
                         new TH2D(Form("m_OscarTel_Ex_ion%s", histoName.c_str()),
                                  Form("OSCAR telescope ID - oscar excitation energy for %s;OSCAR "
                                       "ID;Excitation Energy [MeV]",
                                       histoTitle.c_str()),
                                  128, 0, 128, 50 / 0.05, -10, 40),
                         reactionHistos.ptrs);
    }

    for (const auto &reaction : input.nBodyReactions)
    {
        GammadetectorOscarHistograms::ReactionHistograms &reactionHistos =
            h.reactionHistos[reaction.first];
        std::string histoName;
        std::string histoTitle;

        for (const auto &it : reaction.first)
        {
            histoName += "_";
            histoName += "A" + std::to_string(it.A);
            histoName += "Z" + std::to_string(it.Z);

            histoTitle += " A=" + std::to_string(it.A);
            histoTitle += " Z=" + std::to_string(it.Z);
        }

        reactionHistos.dir = h.dir->mkdir(
            Form("Reaction%s", histoName.c_str()),
            Form("Gamma spectra with cuts on the identified ion: %s", histoTitle.c_str()));
        reactionHistos.dir->cd();

        Initialize<TH1D>(reactionHistos.h_E,
                         new TH1D(Form("h_E_ion%s", histoName.c_str()),
                                  Form("E spectrum for %s;Energy [keV];Counts/%d keV",
                                       histoTitle.c_str(), gammadetectorConf.binWidth),
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV),
                         reactionHistos.ptrs);

        Initialize<TH1D>(reactionHistos.h_EDCBP,
                         new TH1D(Form("h_EDCBP_ion%s", histoName.c_str()),
                                  Form("DC spectrum for %s;Energy DC [keV];Counts/%d keV",
                                       histoTitle.c_str(), gammadetectorConf.binWidth),
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV),
                         reactionHistos.ptrs);

        Initialize<TH2D>(reactionHistos.m_Ex_E,
                         new TH2D(Form("m_Ex_E_ion%s", histoName.c_str()),
                                  Form("Excitation energy spectrum vs gamma energy for "
                                       "%s;Energy [keV];Excitation energy [MeV]",
                                       histoTitle.c_str()),
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV, 1000, -10, 30),
                         reactionHistos.ptrs);

        Initialize<TH2D>(reactionHistos.m_Ex_EDCBP,
                         new TH2D(Form("m_Ex_DCEBP_ion%s", histoName.c_str()),
                                  Form("Excitation energy spectrum vs DC gamma energy for "
                                       "%s;Energy DC [keV];Excitation energy [MeV]",
                                       histoTitle.c_str()),
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV, 1000, -10, 30),
                         reactionHistos.ptrs);
        Initialize<TH2D>(reactionHistos.m_E_E,
                         new TH2D(Form("m_E_E_ion%s", histoName.c_str()),
                                  Form("Gamma-gamma energy for %s;Energy [keV];Energy [keV]",
                                       histoTitle.c_str()),
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV,
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV),
                         reactionHistos.ptrs);

        Initialize<TH2D>(reactionHistos.m_E_ThetaBP,
                         new TH2D(Form("m_E_ThetaBP_ion%s", histoName.c_str()),
                                  Form("Gamma - theta for binary partner of ion %s;Theta [deg];"
                                       "Energy [keV]",
                                       histoTitle.c_str()),
                                  1000, 0, 180,
                                  static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV /
                                                   gammadetectorConf.binWidth),
                                  0, gammadetectorConf.gammaEmax / UNITS::keV),
                         reactionHistos.ptrs);
        Initialize<TH2D>(reactionHistos.m_OscarTel_Ex,
                         new TH2D(Form("m_OscarTel_Ex_ion%s", histoName.c_str()),
                                  Form("OSCAR telescope ID - oscar excitation energy for %s;OSCAR "
                                       "ID;Excitation Energy [MeV]",
                                       histoTitle.c_str()),
                                  128, 0, 128, 50 / 0.05, -10, 40),
                         reactionHistos.ptrs);
    }
    return true;
}

void GammadetectorOscarAnalysis::Clear()
{
    gammaDC.clear();
}

void GammadetectorOscarAnalysis::Analyze()
{
    CoincidenceAnalysis::Analyze();
    gammaDC.clear();
    // if(!in_coinc) return;
    gammaDC.resize(coincMatrix.size());

    for (size_t s{0}; s < coincMatrix.size(); s++)
    {

        auto &currentIon = gammaDC.at(s);
        currentIon.resize(gammadetector.P.size());

        currentIon.p3 = oscar.p3.at(s);
        currentIon.p4 = oscar.p4.at(s);

        for (size_t a{0}; a < currentIon.E.size(); a++)
        {

            if (coincMatrix.at(s).at(a))
            {
                currentIon.inCoinc.at(a) = true;
            }
            else
            {
                currentIon.inCoinc.at(a) = false;
            }

            if (settings.verbosity_level > 8)
            {
                std::cout << "\nGamma " << a << " E X Y Z\t" << gammadetector.E.at(a) << "\t"
                          << gammadetector.Pos.at(a).X() / UNITS::mm << " "
                          << gammadetector.Pos.at(a).Y() / UNITS::mm << " "
                          << gammadetector.Pos.at(a).Z() / UNITS::mm << "\t"
                          << gammadetector.Pos.at(a).Mag() / UNITS::mm << " "
                          << gammadetector.Pos.at(a).Theta() / UNITS::deg << " "
                          << gammadetector.Pos.at(a).Phi() / UNITS::deg << "\n";
                std::cout << "BetaBP " << oscar.p4.at(s).Beta() << "\n";
                std::cout << "costheta "
                          << TMath::Cos(gammadetector.Pos.at(a).Angle(oscar.p4.at(s).Vect()))
                          << std::endl;
                std::cout << "p4.Vect():\t" << oscar.p4.at(s).Vect().Mag() << " "
                          << oscar.p4.at(s).Vect().Theta() / UNITS::deg << " "
                          << oscar.p4.at(s).Vect().Phi() / UNITS::deg << " " << std::endl;
            }

            currentIon.E.at(a)     = gammadetector.P.at(a);
            currentIon.EDC.at(a)   = gammadetector.P.at(a);
            currentIon.EDCBP.at(a) = gammadetector.P.at(a);

            currentIon.EDC.at(a).Boost(-currentIon.p3.BoostVector());
            currentIon.EDCBP.at(a).Boost(-currentIon.p4.BoostVector());
        }
    }
}

bool GammadetectorOscarAnalysis::FillHistos()
{
    if (!CoincidenceAnalysis::FillHistos())
        return false;

    GammadetectorOscarHistograms &h = gammadetectorOscarHistograms;

    for (int s{0}; s < static_cast<int>(oscar.E.size()); ++s)
    {
        // Particle-only observables must be filled once per OSCAR particle, not
        // once for every coincident gamma ray.
        for (auto &selection : h.energyThetaSelections)
        {
            if (!selection.cut->IsInside(oscar.p3.at(s).Theta() / UNITS::deg,
                                         oscar.E.at(s) / UNITS::MeV))
                continue;

            Fill(selection.h_Ex, oscar.Ex.at(s));
            Fill(selection.m_EdE, oscar.ESi.at(s) / UNITS::MeV,
                 oscar.dESi.at(s) / UNITS::MeV);
            Fill(selection.m_Etot_dE, oscar.E.at(s) / UNITS::MeV,
                 oscar.dESi.at(s) / UNITS::MeV);
        }

        for (int a1{0}; a1 < static_cast<int>(gammadetector.E.size()); ++a1)
        {
            if (!gammaDC.at(s).inCoinc.at(a1))
                continue;

            Fill(h.h_E, gammadetector.E.at(a1));
            Fill(h.h_EDC, gammaDC.at(s).EDC.at(a1).E());
            Fill(h.h_EDCBP, gammaDC.at(s).EDCBP.at(a1).E());

            // beta average
            TLorentzVector EDCavg = gammaDC.at(s).E.at(a1);
            EDCavg.Boost(-gammadetectorOscarConf.betaAvg);
            Fill(h.h_EDC_avg, EDCavg.E());

            Fill(h.m_EOscar_Theta_pg, oscar.p3.at(s).Theta() / UNITS::deg,
                 oscar.E.at(s)); // particle-gamma coincidence

            Fill(h.m_E_Beta, gammaDC.at(s).p3.Beta() * 100, gammadetector.E.at(a1));
            Fill(h.m_E_BetaBP, gammaDC.at(s).p4.Beta() * 100, gammadetector.E.at(a1));

            Fill(h.m_EDC_Beta, gammaDC.at(s).p3.Beta() * 100, gammaDC.at(s).EDC.at(a1).E());
            Fill(h.m_EDCBP_BetaBP, gammaDC.at(s).p4.Beta() * 100, gammaDC.at(s).EDCBP.at(a1).E());

            Fill(h.m_E_Theta, oscar.p3.at(s).Angle(gammadetector.P.at(a1).Vect()) / UNITS::deg,
                 gammadetector.E.at(a1));

            Fill(h.m_E_ThetaBP, oscar.p4.at(s).Angle(gammadetector.P.at(a1).Vect()) / UNITS::deg,
                 gammadetector.E.at(a1));

            Fill(h.m_EDC_Theta, oscar.p3.at(s).Angle(gammadetector.P.at(a1).Vect()) / UNITS::deg,
                 gammaDC.at(s).EDC.at(a1).E());
            Fill(h.m_EDCBP_ThetaBP,
                 oscar.p4.at(s).Angle(gammadetector.P.at(a1).Vect()) / UNITS::deg,
                 gammaDC.at(s).EDCBP.at(a1).E());

            Fill(h.m_EDCBP_Theta, oscar.p3.at(s).Angle(gammadetector.P.at(a1).Vect()) / UNITS::deg,
                 gammaDC.at(s).EDCBP.at(a1).E());

            Fill(h.m_OscarID_EDC, oscar.IddE.at(s), gammaDC.at(s).EDC.at(a1).E());
            Fill(h.m_OscarID_EDC, oscar.IdE.at(s), gammaDC.at(s).EDC.at(a1).E());
            Fill(h.m_OscarID_EDCBP, oscar.IddE.at(s), gammaDC.at(s).EDCBP.at(a1).E());
            Fill(h.m_OscarID_EDCBP, oscar.IdE.at(s), gammaDC.at(s).EDCBP.at(a1).E());

            Fill(h.m_EDCBP_TS,
                 unsignedDiff<double, unsigned long long>(oscar.TS, input.firstTS) / 1.E+8,
                 gammaDC.at(s).EDCBP.at(a1).E()); // Added by MR (be careful!)

            Fill(h.m_Ex_E, oscar.Ex.at(s), gammadetector.E.at(a1));
            Fill(h.m_Ex_EDC, oscar.Ex.at(s), gammaDC.at(s).EDC.at(a1).E());
            Fill(h.m_Ex_EDCBP, oscar.Ex.at(s), gammaDC.at(s).EDCBP.at(a1).E());

            for (auto &it : h.exSelection)
            {
                GammadetectorOscarHistograms::ExSelection &hex = it; // Shadowing other reference!
                if ((oscar.Ex.at(s) > hex.vals.first) && (oscar.Ex.at(s) < hex.vals.second))
                {
                    Fill(hex.m_E_Beta, gammaDC.at(s).p3.Beta() * 100, gammadetector.E.at(a1));
                    Fill(hex.m_E_BetaBP, gammaDC.at(s).p4.Beta() * 100, gammadetector.E.at(a1));
                    Fill(hex.h_E, gammadetector.E.at(a1));
                    Fill(hex.h_EDC, gammaDC.at(s).EDC.at(a1).E());
                    Fill(hex.h_EDCBP, gammaDC.at(s).EDCBP.at(a1).E());

                    //		    Fill(hex.m_OscarID_Ex, oscar.Id.at(s),
                    // oscar.Ex.at(s));

                    Fill(hex.m_E_Theta,
                         oscar.p3.at(s).Angle(gammadetector.P.at(a1).Vect()) / UNITS::deg,
                         gammadetector.E.at(a1));
                    Fill(hex.m_E_ThetaBP,
                         oscar.p4.at(s).Angle(gammadetector.P.at(a1).Vect()) / UNITS::deg,
                         gammadetector.E.at(a1));

                    Fill(hex.m_EDCBP_Theta,
                         oscar.p3.at(s).Angle(gammadetector.P.at(a1).Vect()) / UNITS::deg,
                         gammaDC.at(s).EDCBP.at(a1).E());
                    Fill(hex.m_EDCBP_ThetaBP,
                         oscar.p4.at(s).Angle(gammadetector.P.at(a1).Vect()) / UNITS::deg,
                         gammaDC.at(s).EDCBP.at(a1).E());
                }
            }

            for (auto &selection : h.energyThetaSelections)
            {
                if (!selection.cut->IsInside(oscar.p3.at(s).Theta() / UNITS::deg,
                                             oscar.E.at(s) / UNITS::MeV))
                    continue;

                Fill(selection.m_E_Beta, gammaDC.at(s).p3.Beta() * 100,
                     gammadetector.E.at(a1));
                Fill(selection.m_E_BetaBP, gammaDC.at(s).p4.Beta() * 100,
                     gammadetector.E.at(a1));
                Fill(selection.h_E, gammadetector.E.at(a1));
                Fill(selection.h_EDC, gammaDC.at(s).EDC.at(a1).E());
                Fill(selection.h_EDCBP, gammaDC.at(s).EDCBP.at(a1).E());
                Fill(selection.m_E_Theta,
                     oscar.p3.at(s).Angle(gammadetector.P.at(a1).Vect()) / UNITS::deg,
                     gammadetector.E.at(a1));
                Fill(selection.m_E_ThetaBP,
                     oscar.p4.at(s).Angle(gammadetector.P.at(a1).Vect()) / UNITS::deg,
                     gammadetector.E.at(a1));
                Fill(selection.m_EDCBP_Theta,
                     oscar.p3.at(s).Angle(gammadetector.P.at(a1).Vect()) / UNITS::deg,
                     gammaDC.at(s).EDCBP.at(a1).E());
                Fill(selection.m_EDCBP_ThetaBP,
                     oscar.p4.at(s).Angle(gammadetector.P.at(a1).Vect()) / UNITS::deg,
                     gammaDC.at(s).EDCBP.at(a1).E());
            }

            for (int a2 = 0; a2 < a1; ++a2)
            {
                if (!gammaDC.at(s).inCoinc.at(a2))
                    continue;
                Fill(h.m_E_E, gammadetector.E.at(a1), gammadetector.E.at(a2));
                Fill(h.m_E_E, gammadetector.E.at(a2), gammadetector.E.at(a1));

                Fill(h.m_EDC_EDC, gammaDC.at(s).EDC.at(a1).E(), gammaDC.at(s).EDC.at(a2).E());
                Fill(h.m_EDC_EDC, gammaDC.at(s).EDC.at(a2).E(), gammaDC.at(s).EDC.at(a1).E());

                Fill(h.m_EDCBP_EDCBP, gammaDC.at(s).EDCBP.at(a1).E(),
                     gammaDC.at(s).EDCBP.at(a2).E());
                Fill(h.m_EDCBP_EDCBP, gammaDC.at(s).EDCBP.at(a2).E(),
                     gammaDC.at(s).EDCBP.at(a1).E());

                Fill(h.m_E_EDC, gammadetector.E.at(a1), gammaDC.at(s).EDC.at(a2).E());
                Fill(h.m_E_EDC, gammadetector.E.at(a2), gammaDC.at(s).EDC.at(a1).E());

                Fill(h.m_E_EDCBP, gammadetector.E.at(a1), gammaDC.at(s).EDC.at(a2).E());
                Fill(h.m_E_EDCBP, gammadetector.E.at(a2), gammaDC.at(s).EDCBP.at(a1).E());

                Fill(h.m_EDC_EDCBP, gammaDC.at(s).EDC.at(a1).E(), gammaDC.at(s).EDCBP.at(a2).E());
                Fill(h.m_EDC_EDCBP, gammaDC.at(s).EDC.at(a2).E(), gammaDC.at(s).EDCBP.at(a1).E());

                for (auto &it : h.exSelection)
                {
                    GammadetectorOscarHistograms::ExSelection &hex =
                        it; // Shadowing other reference!
                    if ((oscar.Ex.at(s) > hex.vals.first) && (oscar.Ex.at(s) < hex.vals.second))
                    {
                        Fill(hex.m_E_E, gammadetector.E.at(a1), gammadetector.E.at(a2));
                        Fill(hex.m_E_E, gammadetector.E.at(a2), gammadetector.E.at(a1));

                        Fill(hex.m_EDC_EDC, gammaDC.at(s).EDC.at(a1).E(),
                             gammaDC.at(s).EDC.at(a2).E());
                        Fill(hex.m_EDC_EDC, gammaDC.at(s).EDC.at(a2).E(),
                             gammaDC.at(s).EDC.at(a1).E());

                        Fill(hex.m_EDCBP_EDCBP, gammaDC.at(s).EDCBP.at(a1).E(),
                             gammaDC.at(s).EDCBP.at(a2).E());
                        Fill(hex.m_EDCBP_EDCBP, gammaDC.at(s).EDCBP.at(a2).E(),
                             gammaDC.at(s).EDCBP.at(a1).E());

                        Fill(hex.m_E_EDC, gammadetector.E.at(a1), gammaDC.at(s).EDC.at(a2).E());
                        Fill(hex.m_E_EDC, gammadetector.E.at(a2), gammaDC.at(s).EDC.at(a1).E());

                        Fill(hex.m_E_EDCBP, gammadetector.E.at(a1), gammaDC.at(s).EDCBP.at(a2).E());
                        Fill(hex.m_E_EDCBP, gammadetector.E.at(a2), gammaDC.at(s).EDCBP.at(a1).E());

                        Fill(hex.m_EDC_EDCBP, gammaDC.at(s).EDC.at(a1).E(),
                             gammaDC.at(s).EDCBP.at(a2).E());
                        Fill(hex.m_EDC_EDCBP, gammaDC.at(s).EDC.at(a2).E(),
                             gammaDC.at(s).EDCBP.at(a1).E());
                    }
                }

                for (auto &selection : h.energyThetaSelections)
                {
                    if (!selection.cut->IsInside(oscar.p3.at(s).Theta() / UNITS::deg,
                                                 oscar.E.at(s) / UNITS::MeV))
                        continue;

                    Fill(selection.m_E_E, gammadetector.E.at(a1), gammadetector.E.at(a2));
                    Fill(selection.m_E_E, gammadetector.E.at(a2), gammadetector.E.at(a1));
                    Fill(selection.m_EDC_EDC, gammaDC.at(s).EDC.at(a1).E(),
                         gammaDC.at(s).EDC.at(a2).E());
                    Fill(selection.m_EDC_EDC, gammaDC.at(s).EDC.at(a2).E(),
                         gammaDC.at(s).EDC.at(a1).E());
                    Fill(selection.m_EDCBP_EDCBP, gammaDC.at(s).EDCBP.at(a1).E(),
                         gammaDC.at(s).EDCBP.at(a2).E());
                    Fill(selection.m_EDCBP_EDCBP, gammaDC.at(s).EDCBP.at(a2).E(),
                         gammaDC.at(s).EDCBP.at(a1).E());
                    Fill(selection.m_E_EDC, gammadetector.E.at(a1),
                         gammaDC.at(s).EDC.at(a2).E());
                    Fill(selection.m_E_EDC, gammadetector.E.at(a2),
                         gammaDC.at(s).EDC.at(a1).E());
                    Fill(selection.m_E_EDCBP, gammadetector.E.at(a1),
                         gammaDC.at(s).EDCBP.at(a2).E());
                    Fill(selection.m_E_EDCBP, gammadetector.E.at(a2),
                         gammaDC.at(s).EDCBP.at(a1).E());
                    Fill(selection.m_EDC_EDCBP, gammaDC.at(s).EDC.at(a1).E(),
                         gammaDC.at(s).EDCBP.at(a2).E());
                    Fill(selection.m_EDC_EDCBP, gammaDC.at(s).EDC.at(a2).E(),
                         gammaDC.at(s).EDCBP.at(a1).E());
                }
            }
        }
    }

    // Particle identification histograms
    std::vector<ReactionFragment::Ion> reaction;
    for (unsigned int e{0}; e < oscar.E.size(); ++e)
    {
        if (!oscar.identified.at(e))
        {
            continue;
        }

        ReactionFragment::Ion idIon{oscar.A.at(e), oscar.Z.at(e)};
        reaction.emplace_back(idIon);
    }
    std::sort(reaction.begin(), reaction.end());

    auto reactionHist = h.reactionHistos.find(reaction);
    if (reactionHist != h.reactionHistos.end())
    {

        for (unsigned int s{0}; s < gammaDC.size(); ++s)
        {
            for (unsigned int a1{0}; a1 < gammaDC.at(s).E.size(); ++a1)
            {
                const auto ge = gammaDC.at(s).EDCBP.at(a1).E();
                if (ge >= 880 && ge <= 900)
                {
                    Fill(reactionHist->second.m_OscarTel_Ex, oscar.IdTelescope.at(s),
                         oscar.Ex.at(s));
                }
            }

            for (unsigned int a1{0}; a1 < gammaDC.at(s).E.size(); ++a1)
            {
                if (!gammaDC.at(s).inCoinc.at(a1))
                    continue;
                Fill(reactionHist->second.h_E, gammadetector.E.at(a1));
                Fill(reactionHist->second.h_EDCBP, gammaDC.at(s).EDCBP.at(a1).E());

                // beta average
                TLorentzVector EDCavg = gammaDC.at(s).E.at(a1);
                EDCavg.Boost(-gammadetectorOscarConf.betaAvg);
                Fill(reactionHist->second.h_EDC_avg, EDCavg.E());
                Fill(reactionHist->second.m_Ex_E, gammadetector.E.at(a1), oscar.Ex.at(s));
                Fill(reactionHist->second.m_Ex_EDCBP, gammaDC.at(s).EDCBP.at(a1).E(),
                     oscar.Ex.at(s));
                Fill(reactionHist->second.m_E_ThetaBP,
                     oscar.p4.at(s).Angle(gammadetector.P.at(a1).Vect()) / UNITS::deg,
                     gammadetector.E.at(a1));
                for (unsigned int a2{0}; a2 < a1; ++a2)
                {
                    Fill(reactionHist->second.m_E_E, gammadetector.E.at(a1),
                         gammadetector.E.at(a2));
                    Fill(reactionHist->second.m_E_E, gammadetector.E.at(a2),
                         gammadetector.E.at(a1));
                }
            }
        }
    }
    return true;
}

void GammadetectorOscarAnalysis::Finalize()
{
    CoincidenceAnalysis::Finalize();
}

TGraph *GammadetectorOscarAnalysis::GenerateKinematicLine(const double &val)
{
    auto reactionsIt  = input.reactions.find(oscarConf.ion);
    auto reactionCopy = reactionsIt->second; // Do not modify the shared reaction state.
    reactionCopy.GetFreeFragment().Set_Ex(val * UNITS::MeV);

    std::vector<double> x, y;
    double              thetaMax = static_cast<double>(reactionCopy.Get_ThetaMax()) / UNITS::deg;
    x.reserve(180);
    y.reserve(180);

    // Store only the maximum-energy solution. Appending the second solution in
    // reverse theta order makes a single TGraph loop back on itself.
    for (int p{1}; p < thetaMax; ++p)
    {
        try
        {
            reactionCopy.Set_Theta(p * UNITS::deg, true);
        }
        catch (const std::runtime_error &e)
        {
            std::cout << e.what();
            continue;
        }
        x.push_back(p);
        y.push_back(static_cast<double>(reactionCopy.GetFixedFragment().Get_Ek()));
    }
    auto *gr =
        x.empty() ? new TGraph() : new TGraph(static_cast<int>(x.size()), x.data(), y.data());
    gr->SetName(Form("g_Energy_Theta_%.3f_%i", val, kineLineCounter++));
    gr->SetTitle(Form("Kinematic line with Ex = %.3f;Theta [deg];Energy [MeV]", val));
    return gr;
}
