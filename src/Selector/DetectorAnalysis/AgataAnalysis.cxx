#include "AgataAnalysis.h"

#include "Agata.h"

AgataAnalysis::AgataAnalysis(DetectorAnalysis::Inputs in, Agata &a)
    : DetectorAnalysis(in, a, ConfManager::getInstance().agata, agataHistograms), agata(a)
{
    if (detectorConf.enable_treeFill && input.type == Inputs::Type::SIGNAL)
        input.tree->Branch("Agata", &agata);
}

bool AgataAnalysis::AllocateHistos()
{
    AgataHistograms &h = agataHistograms;
    h.dir              = input.outFile.mkdir("Agata", "Agata standalone histograms");
    h.dir->cd();

    if (!DetectorAnalysis::AllocateHistos())
        return false;

    // 1-D histrograms
    Initialize<TH1D>(
        h.h_trackE,
        new TH1D("h_trackE", Form("gamma tracked;Energy [keV];Counts/%d keV", conf.agata.binWidth),
                 static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth), 0,
                 conf.agata.gammaEmax / UNITS::keV),
        h.ptrs);
    Initialize<TH1D>(
        h.h_AddE,
        new TH1D("h_AddE", Form("gamma Addback;Energy [keV];Counts/%d keV", conf.agata.binWidth),
                 static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth), 0,
                 conf.agata.gammaEmax / UNITS::keV),
        h.ptrs);
    Initialize<TH1D>(
        h.h_coreE0,
        new TH1D("h_coreE0", Form("gamma core0;Energy [keV];Counts/%d keV", conf.agata.binWidth),
                 static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth), 0,
                 conf.agata.gammaEmax / UNITS::keV),
        h.ptrs);
    Initialize<TH1D>(
        h.h_coreE1,
        new TH1D("h_coreE1", Form("gamma core1;Energy [keV];Counts/%d keV", conf.agata.binWidth),
                 static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth), 0,
                 conf.agata.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_coreE0_coreID,
        new TH2D("m_coreE0_coreID", "AGATA energy by core;Core ID;Energy [keV];Counts", 100, -0.5,
                 99.5, static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth), 0,
                 conf.agata.gammaEmax / UNITS::keV),
        h.ptrs);
    Initialize<TH2D>(h.m_coreID_time,
                     new TH2D("h_coreID_time",
                              "AGATA Counts in core in time;Time[s];Core ID;Counts", 12 * 180, 0,
                              12 * 3600, 100, -0.5, 99.5),
                     h.ptrs);

    Initialize<TH1D>(h.h_trackTdiff,
                     new TH1D("h_trackTdiff",
                              "Difference between TS of tracks;Time [10 ns];Counts/1ns", 1100, -10,
                              100),
                     h.ptrs);
    Initialize<TH1D>(h.h_coreTdiff,
                     new TH1D("h_coreTdiff",
                              "Difference between TS of cores;Time [10 ns];Counts/10ns", 1110, -10,
                              100),
                     h.ptrs);
    Initialize<TH1D>(h.h_addTdiff,
                     new TH1D("h_addTdiff",
                              "Difference between TS of addbacks;Time [10 ns];Counts/10ns", 1110,
                              -10, 100),
                     h.ptrs);

    // 2-D histrograms
    Initialize<TH2D>(
        h.m_trackE_trackE,
        new TH2D("m_trackE_trackE", "gamma-gamma tracked;Energy [keV];Energy [keV]",
                 static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth), 0,
                 conf.agata.gammaEmax / UNITS::keV,
                 static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth), 0,
                 conf.agata.gammaEmax / UNITS::keV),
        h.ptrs);
    Initialize<TH2D>(
        h.m_AddE_AddE,
        new TH2D("m_AddE_AddE", "gamma-gamma Addback;Energy [keV];Energy [keV]",
                 static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth), 0,
                 conf.agata.gammaEmax / UNITS::keV,
                 static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth), 0,
                 conf.agata.gammaEmax / UNITS::keV),
        h.ptrs);
    Initialize<TH2D>(
        h.m_coreE0_coreE0,
        new TH2D("m_coreE0_coreE0", "gamma-gamma coreE0;Energy [keV];Energy [keV]",
                 static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth), 0,
                 conf.agata.gammaEmax / UNITS::keV,
                 static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth), 0,
                 conf.agata.gammaEmax / UNITS::keV),
        h.ptrs);
    Initialize<TH2D>(
        h.m_coreE1_coreE1,
        new TH2D("m_coreE1_coreE1", "gamma-gamma coreE1;Energy [keV];Energy [keV]",
                 static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth), 0,
                 conf.agata.gammaEmax / UNITS::keV,
                 static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth), 0,
                 conf.agata.gammaEmax / UNITS::keV),
        h.ptrs);

    {
        h.ana.dir = h.dir->mkdir("Ana", "Agata spectra with time condition");
        h.ana.dir->cd();

        Initialize<TH2D>(
            h.ana.m_CR_E,
            new TH2D("m_CR_E",
                     "AGATA Counting rate vs E after passing time "
                     "condition;Time [s];Energy [keV];Counts/s",
                     12 * 360, 0, 12 * 3600,
                     static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth), 0,
                     conf.agata.gammaEmax / UNITS::keV),
            h.ana.ptrs);
        Initialize<TH1D>(h.ana.h_Tdiff,
                         new TH1D("h_Tdiff",
                                  "Difference between TS with time gate ;Time [10 ns];Counts/10ns",
                                  1110, -10, 100),
                         h.ana.ptrs);

        Initialize<TH2D>(
            h.ana.m_layout_XY,
            new TH2D("m_layoutXY", "gamma position XY;X[mm];Y[mm]", 800, -400, 400, 800, -400, 400),
            h.ana.ptrs);
        Initialize<TH2D>(
            h.ana.m_layout_YZ,
            new TH2D("m_layoutYZ", "gamma position YZ;Y[mm];Z[mm]", 800, -400, 400, 800, -400, 400),
            h.ana.ptrs);
        Initialize<TH2D>(
            h.ana.m_layout_XZ,
            new TH2D("m_layoutXZ", "gamma position XZ;X[mm];Z[mm]", 800, -400, 400, 800, -400, 400),
            h.ana.ptrs);
        Initialize<TH2D>(
            h.ana.m_EDC_beta,
            new TH2D("m_EDC_beta", "gamma energy vs beta average;Beta;Energy [keV]", 70,
                     conf.agata.betaAvg.Mag() * 0.5, conf.agata.betaAvg.Mag() * 1.5,
                     static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth), 0,
                     conf.agata.gammaEmax / UNITS::keV),
            h.ana.ptrs);
        Initialize<TH1D>(
            h.ana.h_EDC_avg,
            new TH1D("h_EDC_avg",
                     "Histogram with average Doppler correction;Energy "
                     "[keV];Energy [keV]",
                     static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth), 0,
                     conf.agata.gammaEmax / UNITS::keV),
            h.ana.ptrs);
        Initialize<TH1D>(
            h.ana.h_E,
            new TH1D("h_E",
                     Form("Gamma spectrum (with time conditions, "
                          "h.ana.ptrs);Energy [keV];Counts/%d keV",
                          conf.agata.binWidth),
                     static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth), 0,
                     conf.agata.gammaEmax / UNITS::keV),
            h.ana.ptrs);
        Initialize<TH2D>(
            h.ana.m_E_E,
            new TH2D("m_E_E", "gamma-gamma matrix;Energy [keV];Energy [keV]",
                     static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth), 0,
                     conf.agata.gammaEmax / UNITS::keV,
                     static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth), 0,
                     conf.agata.gammaEmax / UNITS::keV),
            h.ana.ptrs);
        Initialize<TH2D>(
            h.ana.m_EDC_EDC_avg,
            new TH2D("m_EDC_EDC_avg",
                     "gamma-gamma with average Doppler correction;Energy "
                     "[keV];Energy [keV]",
                     static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth), 0,
                     conf.agata.gammaEmax / UNITS::keV,
                     static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth), 0,
                     conf.agata.gammaEmax / UNITS::keV),
            h.ana.ptrs);
        Initialize<TH2D>(
            h.ana.m_E_EDC_avg,
            new TH2D("m_E_EDC_avg",
                     "gamma-gamma Doppler corrected with NON Doppler "
                     "corrected;Energy [keV];Energy DC[keV]",
                     static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth), 0,
                     conf.agata.gammaEmax / UNITS::keV,
                     static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth), 0,
                     conf.agata.gammaEmax / UNITS::keV),
            h.ana.ptrs);
        Initialize<TH2D>(
            h.ana.m_Theta_E_avg,
            new TH2D("m_Theta_E_avg",
                     "Angle with respect to beta avg vector vs gamma "
                     "energy;Angle [deg];Energy [keV]",
                     100, 0, 180,
                     static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth), 0,
                     conf.agata.gammaEmax / UNITS::keV),
            h.ana.ptrs);
        Initialize<TH2D>(
            h.ana.m_Theta_EDC_avg,
            new TH2D("m_Theta_EDC_avg",
                     "Angle with respect to beta avg vector vs Doppler "
                     "corrected gamma energy;Angle [deg];Energy DC[keV]",
                     100, 0, 180,
                     static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth), 0,
                     conf.agata.gammaEmax / UNITS::keV),
            h.ana.ptrs);

        // 3D histograms
        Initialize<TH3D>(h.ana.m_layout_XYZ,
                         new TH3D("m_layoutXYZ", "gamma position XYZ;X[mm];Y[mm];Z[mm]", 200, -400,
                                  400, 200, -400, 400, 200, -400, 400),
                         h.ana.ptrs);

        // Histograms with conditions on gamma energy
        for (unsigned int i{0}; i < conf.agata.gammaGates.size(); ++i)
        {
            h.ana.gammagatedHistograms.emplace_back();
            auto &vals = conf.agata.gammaGates.at(i);

            h.ana.gammagatedHistograms.back().vals   = vals.gate;
            h.ana.gammagatedHistograms.back().option = vals.option;
            h.ana.gammagatedHistograms.back().dir    = h.ana.dir->mkdir(
                Form("GammaGate_%i_%i_%s", vals.gate.first, vals.gate.second, vals.option.c_str()),
                Form("Gamma spectra gated on gamma line, E=%i-%i, %s", vals.gate.first,
                        vals.gate.second, vals.option.c_str()));
            h.ana.gammagatedHistograms.back().exclude_angle = vals.exclude_angle;

            //	  std::cout << "Gate" <<
            // h.ana.gammagatedHistograms.back().vals.first << std::endl;

            // *************************************************** //
            // ********** GAMMA GATE ***************************** //

            h.ana.gammagatedHistograms.back().dir->cd();

            Initialize<TH1D>(
                h.ana.gammagatedHistograms.back().h_E,
                new TH1D("h_E",
                         Form("Gamma spectrum (with time conditions, "
                              "h.ana.gammagatedHistograms.back().ptrs);Energy "
                              "[keV];Counts/%d keV",
                              conf.agata.binWidth),
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV),
                h.ana.gammagatedHistograms.back().ptrs);

            Initialize<TH1D>(
                h.ana.gammagatedHistograms.back().h_EDC_avg,
                new TH1D("h_EDC_avg",
                         "Histogram with average Doppler correction;Energy "
                         "[keV];Energy [keV]",
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV),
                h.ana.gammagatedHistograms.back().ptrs);

            Initialize<TH2D>(
                h.ana.gammagatedHistograms.back().m_E_E,
                new TH2D("m_E_E", "gamma-gamma matrix;Energy [keV];Energy [keV]",
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV,
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV),
                h.ana.gammagatedHistograms.back().ptrs);

            Initialize<TH1D>(
                h.ana.gammagatedHistograms.back().h_AddE_mult,
                new TH1D("h_AddE_mult", "Average multiplicity for add-back;Mult;Counts", 20, 0, 20),
                h.ana.gammagatedHistograms.back().ptrs);

            Initialize<TH2D>(
                h.ana.gammagatedHistograms.back().m_AddE_AddE,
                new TH2D("m_AddE_AddE", "gamma-gamma Addback;Energy [keV];Energy [keV]",
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV,
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV),
                h.ana.gammagatedHistograms.back().ptrs);

            Initialize<TH2D>(
                h.ana.gammagatedHistograms.back().m_AddE_AddE_mult2,
                new TH2D("m_AddE_AddE_mult2",
                         "gamma-gamma Addback mult=2;Energy [keV];Energy [keV]",
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV,
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV),
                h.ana.gammagatedHistograms.back().ptrs);

            Initialize<TH2D>(
                h.ana.gammagatedHistograms.back().m_AddE_AddE_mult3,
                new TH2D("m_AddE_AddE_mult3",
                         "gamma-gamma Addback mult=3;Energy [keV];Energy [keV]",
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV,
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV),
                h.ana.gammagatedHistograms.back().ptrs);

            Initialize<TH2D>(
                h.ana.gammagatedHistograms.back().m_AddE_AddE_mult4,
                new TH2D("m_AddE_AddE_mult4",
                         "gamma-gamma Addback mult=4;Energy [keV];Energy [keV]",
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV,
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV),
                h.ana.gammagatedHistograms.back().ptrs);

            Initialize<TH2D>(
                h.ana.gammagatedHistograms.back().m_EDC_EDC_avg,
                new TH2D("m_EDC_EDC_avg",
                         "gamma-gamma with average Doppler correction;Energy "
                         "[keV];Energy [keV]",
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV,
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV),
                h.ana.gammagatedHistograms.back().ptrs);

            Initialize<TH2D>(
                h.ana.gammagatedHistograms.back().m_E_EDC_avg,
                new TH2D("m_E_EDC_avg",
                         "gamma-gamma Doppler corrected with NON Doppler "
                         "corrected;Energy [keV];Energy DC[keV]",
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV,
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV),
                h.ana.gammagatedHistograms.back().ptrs);

            Initialize<TH2D>(
                h.ana.gammagatedHistograms.back().m_Theta_E_avg,
                new TH2D("m_Theta_E_avg",
                         "Angle with respect to beta avg vector vs gamma "
                         "energy;Angle [deg];Energy [keV]",
                         100, 0, 180,
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV),
                h.ana.gammagatedHistograms.back().ptrs);

            Initialize<TH2D>(
                h.ana.gammagatedHistograms.back().m_Theta_EDC_avg,
                new TH2D("m_Theta_EDC_avg",
                         "Angle with respect to beta avg vector vs Doppler "
                         "corrected gamma energy;Angle [deg];Energy DC[keV]",
                         100, 0, 180,
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV),
                h.ana.gammagatedHistograms.back().ptrs);
        }

        // spectra in anticoincidence
        if (conf.agata.excludeAncillary)
        {
            h.ana.anti.dir =
                h.ana.dir->mkdir("Ana", "Agata spectra in anticoincidence with ancillaries");
            h.ana.anti.dir->cd();
            Initialize<TH1D>(h.ana.anti.h_CR,
                             new TH1D("h_CR",
                                      "AGATA Counting rate after passing time "
                                      "condition;Time [s];Counts/s",
                                      12 * 360, 0, 12 * 3600),
                             h.ana.anti.ptrs);

            Initialize<TH2D>(
                h.ana.anti.m_CR_E,
                new TH2D("m_CR_E",
                         "AGATA Counting rate vs E after passing time "
                         "condition;Time [s];Energy [keV];Counts/s",
                         12 * 360, 0, 12 * 3600,
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV),
                h.ana.anti.ptrs);

            Initialize<TH2D>(h.ana.anti.m_layout_XY,
                             new TH2D("m_layoutXY", "gamma position XY;X[mm];Y[mm]", 800, -400, 400,
                                      800, -400, 400),
                             h.ana.anti.ptrs);
            Initialize<TH2D>(h.ana.anti.m_layout_YZ,
                             new TH2D("m_layoutYZ", "gamma position YZ;Y[mm];Z[mm]", 800, -400, 400,
                                      800, -400, 400),
                             h.ana.anti.ptrs);
            Initialize<TH2D>(h.ana.anti.m_layout_XZ,
                             new TH2D("m_layoutXZ", "gamma position XZ;X[mm];Z[mm]", 800, -400, 400,
                                      800, -400, 400),
                             h.ana.anti.ptrs);
            Initialize<TH2D>(
                h.ana.anti.m_EDC_beta,
                new TH2D("m_EDC_beta", "gamma energy vs beta average;Beta;Energy [keV]", 70,
                         conf.agata.betaAvg.Mag() * 0.5, conf.agata.betaAvg.Mag() * 1.2,
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV),
                h.ana.anti.ptrs);
            Initialize<TH1D>(
                h.ana.anti.h_EDC_avg,
                new TH1D("h_EDC_avg",
                         "Histogram with average Doppler correction;Energy "
                         "[keV];Energy [keV]",
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV),
                h.ana.anti.ptrs);
            Initialize<TH1D>(
                h.ana.anti.h_E,
                new TH1D("h_E",
                         Form("Gamma spectrum (with time conditions, "
                              "h.ana.anti.ptrs);Energy [keV];Counts/%d keV",
                              conf.agata.binWidth),
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV),
                h.ana.anti.ptrs);
            Initialize<TH2D>(
                h.ana.anti.m_E_E,
                new TH2D("m_E_E", "gamma-gamma matrix;Energy [keV];Energy [keV]",
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV,
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV),
                h.ana.anti.ptrs);
            Initialize<TH2D>(
                h.ana.anti.m_EDC_EDC_avg,
                new TH2D("m_EDC_EDC_avg",
                         "gamma-gamma with average Doppler correction;Energy "
                         "[keV];Energy [keV]",
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV,
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV),
                h.ana.anti.ptrs);
            Initialize<TH2D>(
                h.ana.anti.m_E_EDC_avg,
                new TH2D("m_E_EDC_avg",
                         "gamma-gamma Doppler corrected with NON Doppler "
                         "corrected;Energy [keV];Energy DC[keV]",
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV,
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV),
                h.ana.anti.ptrs);
            Initialize<TH2D>(
                h.ana.anti.m_Theta_E_avg,
                new TH2D("m_Theta_E_avg",
                         "Angle with respect to beta avg vector vs gamma "
                         "energy;Angle [deg];Energy [keV]",
                         100, 0, 180,
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV),
                h.ana.anti.ptrs);
            Initialize<TH2D>(
                h.ana.anti.m_Theta_EDC_avg,
                new TH2D("m_Theta_EDC_avg",
                         "Angle with respect to beta avg vector vs Doppler "
                         "corrected gamma energy;Angle [deg];Energy DC[keV]",
                         100, 0, 180,
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV),
                h.ana.anti.ptrs);
        }
    }
    return true;
}

void AgataAnalysis::Analyze()
{
    DetectorAnalysis::Analyze();

    double lowestT{std::numeric_limits<double>::max()};
    size_t lowestIdx{0};

    // Fill Agata tree
    auto ana = [&](ULong64_t &TS, std::vector<double> &e, std::vector<double> &t,
                   std::vector<double> &x, std::vector<double> &y, std::vector<double> &z,
                   std::vector<Int_t> &core) {
        for (size_t i{0}; i < e.size(); ++i)
        {
            if (lowestT > t.at(i))
            {
                lowestT   = t.at(i);
                lowestIdx = i;
            }
        }

        agata.TS = TS;
        std::vector<unsigned int> idx;
        for (size_t i{0}; i < e.size(); ++i)
        {
            if (t.at(i) - lowestT < windowRight && t.at(i) - lowestT >= windowLeft)
                idx.push_back(i);
        }
        agata.resize(idx.size());
        for (size_t i{0}; i < idx.size(); ++i)
        {
            agata.E[i]      = e.at(idx[i]);
            agata.CoreID[i] = core.at(idx[i]);
            agata.Time[i]   = (t.at(idx[i])) * conf.agata.cfd_unit;
            agata.Pos[i]    = TVector3(y.at(idx[i]) * UNITS::mm, -x.at(idx[i]) * UNITS::mm,
                                       z.at(idx[i]) * UNITS::mm);
        }
        return idx;
    };

    ULong64_t           TS{std::numeric_limits<ULong64_t>::max()};
    std::vector<double> e;
    std::vector<double> t;
    std::vector<double> x;
    std::vector<double> y;
    std::vector<double> z;
    std::vector<int>    core;
    switch (conf.agata.spectraType)
    {
    case AgataConf::SpectraType::SEGMENT: {
        e.resize(**agataReader.nbHits);
        t.resize(**agataReader.nbHits);
        x.resize(**agataReader.nbHits);
        y.resize(**agataReader.nbHits);
        z.resize(**agataReader.nbHits);
        core.resize(**agataReader.nbHits);
        for (int i{0}; i < **agataReader.nbHits; ++i)
        {
            e.at(i)    = agataReader.hitE->At(i);
            t.at(i)    = agataReader.hitT->At(i);
            x.at(i)    = agataReader.hitX->At(i);
            y.at(i)    = agataReader.hitY->At(i);
            z.at(i)    = agataReader.hitZ->At(i);
            core.at(i) = agataReader.hitId->At(i);
        }
        TS = **agataReader.TSHit;

        break;
    }
    case AgataConf::SpectraType::CORE: {
        e.resize(**agataReader.nbCores);
        t.resize(**agataReader.nbCores);
        x.resize(**agataReader.nbCores);
        y.resize(**agataReader.nbCores);
        z.resize(**agataReader.nbCores);
        core.resize(**agataReader.nbCores);
        for (int i{0}; i < **agataReader.nbCores; ++i)
        {
            if (TS > agataReader.coreTS->At(i))
            {
                TS = agataReader.coreTS->At(i);
            }
        }

        for (int i{0}; i < **agataReader.nbCores; ++i)
        {
            e.at(i) = agataReader.coreE0->At(i);

            t.at(i) = unsignedDiff<double, unsigned long long>(agataReader.coreTS->At(i), TS) +
                      agataReader.coreT0->At(i) -
                      39.5; //-39.5 is due to the fixed cfd delay of agata
            core.at(i) = agataReader.coreId->At(i);
            float maxEneHit{0};
            for (int j{0}; j < **agataReader.nbHits; ++j)
            {
                if (agataReader.hitId->At(j) == agataReader.coreId->At(i) &&
                    agataReader.hitE->At(j) > maxEneHit)
                {
                    maxEneHit = agataReader.hitE->At(j);
                    x.at(i)   = agataReader.hitGX->At(j);
                    y.at(i)   = agataReader.hitGY->At(j);
                    z.at(i)   = agataReader.hitGZ->At(j);
                }
            }
        }
        break;
    }
    case AgataConf::SpectraType::CORE1: {
        e.resize(**agataReader.nbCores);
        t.resize(**agataReader.nbCores);
        x.resize(**agataReader.nbCores);
        y.resize(**agataReader.nbCores);
        z.resize(**agataReader.nbCores);
        core.resize(**agataReader.nbCores);
        for (int i{0}; i < **agataReader.nbCores; ++i)
        {
            if (TS > agataReader.coreTS->At(i))
            {
                TS = agataReader.coreTS->At(i);
            }
        }

        for (int i{0}; i < **agataReader.nbCores; ++i)
        {
            e.at(i) = agataReader.coreE1->At(i);

            t.at(i) = unsignedDiff<double, unsigned long long>(agataReader.coreTS->At(i), TS) +
                      agataReader.coreT0->At(i) -
                      39.5; //-39.5 is due to the fixed cfd delay of agata
            core.at(i) = agataReader.coreId->At(i);
            float maxEneHit{0};
            for (int j{0}; j < **agataReader.nbHits; ++j)
            {
                if (agataReader.hitId->At(j) == agataReader.coreId->At(i) &&
                    agataReader.hitE->At(j) > maxEneHit)
                {
                    maxEneHit = agataReader.hitE->At(j);
                    x.at(i)   = agataReader.hitGX->At(j);
                    y.at(i)   = agataReader.hitGY->At(j);
                    z.at(i)   = agataReader.hitGZ->At(j);
                }
            }
        }
        break;
    }
    case AgataConf::SpectraType::ADDBACK: {
        e.resize(**agataReader.nbAdd);
        t.resize(**agataReader.nbAdd);
        x.resize(**agataReader.nbAdd);
        y.resize(**agataReader.nbAdd);
        z.resize(**agataReader.nbAdd);
        core.resize(**agataReader.nbAdd);
        for (int i{0}; i < **agataReader.nbAdd; ++i)
        {
            if (TS > agataReader.AddTS->At(i))
            {
                TS = agataReader.AddTS->At(i);
            }
        }

        for (int i{0}; i < **agataReader.nbAdd; ++i)
        {
            e.at(i) = agataReader.AddE->At(i);
            t.at(i) = unsignedDiff<double, unsigned long long>(agataReader.AddTS->At(i), TS);
            for (int c{0}; c < **agataReader.nbCores; ++c)
            {

                if (agataReader.coreId->At(c) == agataReader.AddId->At(i))
                {
                    t.at(i) += agataReader.coreT0->At(c) -
                               39.5; //-39.5 is due to the fixed cfd delay of agata
                    break;
                }
            }
            core.at(i) = agataReader.AddId->At(i);
            x.at(i)    = agataReader.AddX->At(i);
            y.at(i)    = agataReader.AddY->At(i);
            z.at(i)    = agataReader.AddZ->At(i);
        }
        break;
    }
    case AgataConf::SpectraType::TRACKED: {
        if (agataReader.trackE == nullptr)
            throw std::runtime_error("Tracking selected but not present in input trees");
        e.reserve(**agataReader.nbTrack);
        t.reserve(**agataReader.nbTrack);
        x.reserve(**agataReader.nbTrack);
        y.reserve(**agataReader.nbTrack);
        z.reserve(**agataReader.nbTrack);
        core.reserve(**agataReader.nbTrack);
        for (int i{0}; i < **agataReader.nbTrack; ++i)
        {
            if (agataReader.trackFOM->At(i) > conf.agata.maxFOMThreshold ||
                agataReader.trackFOM->At(i) < conf.agata.minFOMThreshold)
            {
                continue;
            }
            if (conf.agata.trackType > 0 && agataReader.trackType->At(i) != conf.agata.trackType)
            {
                continue;
            }

            e.emplace_back(agataReader.trackE->At(i));
            t.emplace_back(agataReader.trackT->At(i) -
                           39.5); //-39.5 is due to the fixed cfd delay of agata
            core.emplace_back(agataReader.trackCrystalID->At(i));
            x.emplace_back(agataReader.trackX1->At(i));
            y.emplace_back(agataReader.trackY1->At(i));
            z.emplace_back(agataReader.trackZ1->At(i));
        }
        TS = **agataReader.TStrack;
        break;
    }
    case AgataConf::SpectraType::CALORIMETER: {
        e.resize(1, 0);
        t.resize(1, 0);
        x.resize(1, 0);
        y.resize(1, 0);
        z.resize(1, 0);
        core.resize(1, 0);
        double maxCoreEne{0};
        for (int i{0}; i < **agataReader.nbCores; ++i)
        {
            if (agataReader.coreTS->At(i) < TS)
            {
                TS = agataReader.coreTS->At(i);
                t.at(0) =
                    agataReader.coreT0->At(i) - 39.5; //-39.5 is due to the fixed cfd delay of agata
            }
        }
        for (int i{0}; i < **agataReader.nbCores; ++i)
        {
            auto diff = unsignedDiff<double, unsigned long long>(agataReader.coreTS->At(i), TS);
            +(t.at(0) - agataReader.coreT0->At(i));
            if (diff > windowRight)
            {
                continue;
            }
            e.at(0) += agataReader.coreE0->At(i);
            if (agataReader.coreE0->At(i) > maxCoreEne)
            {
                double maxHitEne{0};
                for (int j{0}; j < **agataReader.nbHits; ++j)
                {
                    if (agataReader.hitE->At(j) < maxHitEne)
                    {
                        continue;
                    }
                    maxHitEne = agataReader.hitE->At(j);

                    x.at(0)    = agataReader.hitGX->At(i);
                    y.at(0)    = agataReader.hitGY->At(i);
                    z.at(0)    = agataReader.hitGZ->At(i);
                    core.at(0) = agataReader.hitId->At(i);
                }
                maxCoreEne = agataReader.coreE0->At(i);
            }
        }
        TS = **agataReader.TStrack;
        break;
    }
    default:
        throw std::runtime_error("Something wrong with Agata spectrum type\n");
    }
    if (TS != std::numeric_limits<ULong64_t>::max())
    {
        ana(TS, e, t, x, y, z, core);
    }

    for (unsigned int i{0}; i < agata.E.size(); ++i)
    {
        // Small variations
        agata.Pos[i] += TVector3(conf.agata.xShift, conf.agata.yShift, conf.agata.zShift);
        agata.Pos[i].RotateZ(conf.agata.phi);
        agata.Pos[i].RotateX(conf.agata.thetaX);
        agata.Pos[i].RotateY(conf.agata.thetaY);

        double angleAgata = conf.agata.angle;
        if (conf.detectorsPresent.Prisma)
        {
            if (conf.prisma.agata_opposite)
                angleAgata = UNITS::CONSTANTS::pi - conf.prisma.angle;
        }
        // Major variation
        agata.Pos[i].RotateY(angleAgata - UNITS::CONSTANTS::pi);

        agata.P[i] = TLorentzVector(0, 0, agata.E.at(i), agata.E.at(i));
        agata.P[i].SetTheta(agata.Pos.at(i).Theta());
        agata.P[i].SetPhi(agata.Pos.at(i).Phi());
    }
}

bool AgataAnalysis::FillHistos()
{
    if (!DetectorAnalysis::FillHistos())
        return false;

    // Histograms before analysis
    AgataHistograms &h = agataHistograms;

    double lowesttrackT{std::numeric_limits<double>::max()};
    double lowestcoreT{std::numeric_limits<double>::max()};
    double lowestaddT{std::numeric_limits<double>::max()};
    int    lowesttrackIdx{0};
    int    lowestcoreIdx{0};
    int    lowestaddIdx{0};

    // Tracked loop
    if (agataReader.trackE != nullptr)
    {
        for (int i{0}; i < **agataReader.nbTrack; ++i)
        {
            if (lowesttrackT > agataReader.trackT->At(i) - 39.5)
            {
                lowesttrackT   = agataReader.trackT->At(i) - 39.5;
                lowesttrackIdx = i;
            }
        }
        for (int i{0}; i < **agataReader.nbTrack; ++i)
        {
            Fill(h.h_trackE, agataReader.trackE->At(i));
            if (i != lowesttrackIdx)
                Fill(h.h_trackTdiff, agataReader.trackT->At(i) - 39.5 - lowesttrackT);
            for (int j{0}; j < i; ++j)
            {
                Fill(h.m_trackE_trackE, agataReader.trackE->At(i), agataReader.trackE->At(j));
                Fill(h.m_trackE_trackE, agataReader.trackE->At(j), agataReader.trackE->At(i));
            }
        }
    }
    // Addback loop
    ULong64_t lowestaddTS{std::numeric_limits<ULong64_t>::max()};
    for (int i{0}; i < **agataReader.nbAdd; ++i)
    {
        if (lowestaddTS > agataReader.AddTS->At(i))
        {
            lowestaddTS = agataReader.AddTS->At(i);
        }
    }
    for (int i{0}; i < **agataReader.nbAdd; ++i)
    {
        auto addT = unsignedDiff<double, unsigned long long>(agataReader.AddTS->At(i), lowestaddTS);
        for (int c{0}; c < **agataReader.nbCores; ++c)
        {

            if (agataReader.coreId->At(c) == agataReader.AddId->At(i))
            {
                addT += agataReader.coreT0->At(c) - 39.5;
                break;
            }
        }
        if (lowestaddT > addT)
        {
            lowestaddT   = addT;
            lowestaddIdx = i;
        }
    }
    for (int i{0}; i < **agataReader.nbAdd; ++i)
    {
        if (i != lowestaddIdx)
        {
            auto addT =
                unsignedDiff<double, unsigned long long>(agataReader.AddTS->At(i), lowestaddTS);

            for (int c{0}; c < **agataReader.nbCores; ++c)
            {
                if (agataReader.coreId->At(c) == agataReader.AddId->At(i))
                {
                    addT += agataReader.coreT0->At(c) - 39.5;
                    break;
                }
            }
            Fill(h.h_addTdiff, addT - lowestaddT);
        }
    }
    for (int i{0}; i < **agataReader.nbAdd; ++i)
    {
        Fill(h.h_AddE, agataReader.AddE->At(i));
        for (int j{0}; j < i; ++j)
        {
            Fill(h.m_AddE_AddE, agataReader.AddE->At(i), agataReader.AddE->At(j));
            Fill(h.m_AddE_AddE, agataReader.AddE->At(j), agataReader.AddE->At(i));
        }
    }
    // Core loop
    ULong64_t lowestcoreTS{std::numeric_limits<ULong64_t>::max()};
    for (int i{0}; i < **agataReader.nbCores; ++i)
    {
        if (lowestcoreTS > agataReader.coreTS->At(i))
        {
            lowestcoreTS = agataReader.coreTS->At(i);
        }
    }
    for (int i{0}; i < **agataReader.nbCores; ++i)
    {
        double coreT =
            unsignedDiff<double, unsigned long long>(agataReader.coreTS->At(i), lowestcoreTS) +
            agataReader.coreT0->At(i) - 39.5;
        if (lowestcoreT > coreT)
        {
            lowestcoreT   = coreT;
            lowestcoreIdx = i;
        }
    }
    for (int i{0}; i < **agataReader.nbCores; ++i)
    {
        double coreT =
            unsignedDiff<double, unsigned long long>(agataReader.coreTS->At(i), lowestcoreTS) +
            agataReader.coreT0->At(i) - 39.5;
        if (i != lowestcoreIdx)
            Fill(h.h_coreTdiff, coreT - lowestcoreT);
        Fill(h.m_coreE0_coreID, agataReader.coreId->At(i), agataReader.coreE0->At(i));
        Fill(h.m_coreID_time,
             unsignedDiff<double, unsigned long long>(agataReader.coreTS->At(i), input.firstTS) /
                 1.e8,
             agataReader.coreId->At(i));
        Fill(h.h_coreE0, agataReader.coreE0->At(i));
        Fill(h.h_coreE1, agataReader.coreE1->At(i));
        for (int j{0}; j < i; ++j)
        {
            Fill(h.m_coreE0_coreE0, agataReader.coreE0->At(i), agataReader.coreE0->At(j));
            Fill(h.m_coreE0_coreE0, agataReader.coreE0->At(j), agataReader.coreE0->At(i));

            Fill(h.m_coreE1_coreE1, agataReader.coreE1->At(i), agataReader.coreE1->At(j));
            Fill(h.m_coreE1_coreE1, agataReader.coreE1->At(j), agataReader.coreE1->At(i));
        }
    }

    // Fill of agata stuff
    AgataHistograms::AgataAnaHistograms &ha = h.ana;
    std::vector<TLorentzVector>          pDC;
    bool                                 inRegion = false;

    for (auto exTS : conf.agata.excludeTimestamp)
    {
        if (unsignedDiff<double, unsigned long long>(agata.TS, input.firstTS) / 1.e5 > exTS.first &&
            unsignedDiff<double, unsigned long long>(agata.TS, input.firstTS) / 1.e5 <
                exTS.second) // only in millisecond sorry
            inRegion = true;
    }

    int    id_minT = 0;
    double minT    = std::numeric_limits<double>::max();
    for (unsigned int i{0}; i < agata.E.size(); ++i)
    {
        if (agata.Time.at(i) < minT)
        {
            minT    = agata.Time.at(i);
            id_minT = i;
        }
    }
    for (unsigned int i{0}; i < agata.E.size(); ++i)
    {
        if (inRegion)
            continue;
        // 1D haistograms
        if (ha.m_CR_E)
        {
            Fill(ha.m_CR_E,
                 unsignedDiff<double, unsigned long long>(agata.TS, input.firstTS) / 1.e8,
                 agata.E.at(i), 1. / ha.m_CR_E->GetXaxis()->GetBinWidth(1));
        }

        if (i != static_cast<unsigned int>(id_minT))
            Fill(ha.h_Tdiff, (agata.Time.at(i) - minT) / (10 * UNITS::ns));
        Fill(ha.m_layout_XY, agata.Pos.at(i).x() / UNITS::mm, agata.Pos.at(i).y() / UNITS::mm);
        Fill(ha.m_layout_XZ, agata.Pos.at(i).x() / UNITS::mm, agata.Pos.at(i).z() / UNITS::mm);
        Fill(ha.m_layout_YZ, agata.Pos.at(i).y() / UNITS::mm, agata.Pos.at(i).z() / UNITS::mm);
        Fill(ha.m_layout_XYZ, agata.Pos.at(i).x() / UNITS::mm, agata.Pos.at(i).y() / UNITS::mm,
             agata.Pos.at(i).z() / UNITS::mm);

        Fill(ha.h_E, agata.E.at(i));

        pDC.push_back(agata.P.at(i));
        if (conf.agata.betaAvg.Mag() != 0)
            pDC.back().Boost(-conf.agata.betaAvg);

        Fill(ha.h_EDC_avg, pDC.back().E());

        if (conf.agata.betaAvg.Mag() != 0)
        {
            Fill(ha.m_Theta_E_avg, conf.agata.betaAvg.Angle(agata.Pos.at(i)) / UNITS::deg,
                 agata.E.at(i));
            Fill(ha.m_Theta_EDC_avg, conf.agata.betaAvg.Angle(agata.Pos.at(i)) / UNITS::deg,
                 pDC.back().E());
        }
        else
        {
            Fill(ha.m_Theta_E_avg, agata.Pos.at(i).Theta() / UNITS::deg, agata.E.at(i));
            Fill(ha.m_Theta_EDC_avg, agata.Pos.at(i).Theta() / UNITS::deg, pDC.back().E());
        }

        // necessary to do this way because pDC is filled along with the i
        // cycle, so pDC.size()<agata.size()
        //  Symmetric loop
        for (unsigned int j{0}; j < i; ++j)
        {
            Fill(ha.m_EDC_EDC_avg, pDC.at(i).E(), pDC.at(j).E());
            Fill(ha.m_EDC_EDC_avg, pDC.at(j).E(), pDC.at(i).E());

            Fill(ha.m_E_E, agata.E.at(i), agata.E.at(j));
            Fill(ha.m_E_E, agata.E.at(j), agata.E.at(i));
        }
        // Asymmetric loop
        for (unsigned int j{0}; j < agata.E.size(); ++j)
        {
            if (i != j)
                Fill(ha.m_E_EDC_avg, agata.E.at(j), pDC.at(i).E());
        }

        if (ha.m_EDC_beta && conf.agata.betaAvg.Mag() != 0)
        {
            for (int b{0}; b < ha.m_EDC_beta->GetXaxis()->GetNbins(); b++)
            {
                TLorentzVector p    = agata.P[i];
                TVector3       beta = conf.agata.betaAvg;
                beta.SetMag(ha.m_EDC_beta->GetXaxis()->GetBinCenter(b));
                p.Boost(-beta);
                Fill(ha.m_EDC_beta, beta.Mag(), p.E());
            }
        }

        // ANTICOINCIDENCE ANALYSIS

        // bool coincidencewithAnc =  (agata.prisma_coinc[i] ||
        // agata.dante_coinc[i] || agata.spider_coinc[i]  ||
        // agata.euclides_coinc[i]);
        bool coincidencewithAnc = false;

        if (conf.agata.excludeAncillary && !coincidencewithAnc)
        {
            AgataHistograms::AgataAnaHistograms::AgataAntiHistograms &antihisto = h.ana.anti;
            // 1D haistograms
            if (antihisto.h_CR != nullptr)
            {
                Fill(antihisto.h_CR,
                     unsignedDiff<double, unsigned long long>(agata.TS, input.firstTS) / 1.e8,
                     1. / antihisto.h_CR->GetBinWidth(1));
            }
            if (antihisto.m_CR_E)
            {
                Fill(antihisto.m_CR_E,
                     unsignedDiff<double, unsigned long long>(agata.TS, input.firstTS) / 1.e8,
                     agata.E.at(i), 1. / antihisto.m_CR_E->GetXaxis()->GetBinWidth(1));
            }

            Fill(antihisto.m_layout_XY, agata.Pos.at(i).x() / UNITS::mm,
                 agata.Pos.at(i).y() / UNITS::mm);
            Fill(antihisto.m_layout_XZ, agata.Pos.at(i).x() / UNITS::mm,
                 agata.Pos.at(i).z() / UNITS::mm);
            Fill(antihisto.m_layout_YZ, agata.Pos.at(i).y() / UNITS::mm,
                 agata.Pos.at(i).z() / UNITS::mm);
            Fill(antihisto.m_layout_XYZ, agata.Pos.at(i).x() / UNITS::mm,
                 agata.Pos.at(i).y() / UNITS::mm, agata.Pos.at(i).z() / UNITS::mm);

            Fill(antihisto.h_E, agata.E.at(i));
            // pDC.push_back(agata.P.at(i));
            // if(conf.agata.betaAvg.Mag() != 0)
            //  pDC.back().Boost(-conf.agata.betaAvg);
            // since we are in the analyzed part, this vector should have
            // already been filled

            Fill(antihisto.h_EDC_avg, pDC.back().E());

            if (conf.agata.betaAvg.Mag() != 0)
            {
                Fill(antihisto.m_Theta_E_avg,
                     conf.agata.betaAvg.Angle(agata.Pos.at(i)) / UNITS::deg, agata.E.at(i));
                Fill(antihisto.m_Theta_EDC_avg,
                     conf.agata.betaAvg.Angle(agata.Pos.at(i)) / UNITS::deg, pDC.back().E());
            }
            else
            {
                Fill(antihisto.m_Theta_E_avg, agata.Pos.at(i).Theta() / UNITS::deg, agata.E.at(i));
                Fill(antihisto.m_Theta_EDC_avg, agata.Pos.at(i).Theta() / UNITS::deg,
                     pDC.back().E());
            }

            // necessary to do this way because pDC is filled along with the i
            // cycle, so pDC.size()<agata.size()
            //  Symmetric loop
            for (unsigned int j{0}; j < i; ++j)
            {
                Fill(antihisto.m_EDC_EDC_avg, pDC.at(i).E(), pDC.at(j).E());
                Fill(antihisto.m_EDC_EDC_avg, pDC.at(j).E(), pDC.at(i).E());

                Fill(antihisto.m_E_E, agata.E.at(i), agata.E.at(j));
                Fill(antihisto.m_E_E, agata.E.at(j), agata.E.at(i));
            }
            // Asymmetric loop
            for (unsigned int j{0}; j < agata.E.size(); ++j)
            {
                if (i != j)
                    Fill(antihisto.m_E_EDC_avg, agata.E.at(j), pDC.at(i).E());
            }

            if (antihisto.m_EDC_beta && conf.agata.betaAvg.Mag() != 0)
            {
                for (int b{0}; b < antihisto.m_EDC_beta->GetXaxis()->GetNbins(); b++)
                {
                    TLorentzVector p    = agata.P[i];
                    TVector3       beta = conf.agata.betaAvg;
                    beta.SetMag(antihisto.m_EDC_beta->GetXaxis()->GetBinCenter(b));
                    p.Boost(-beta);
                    Fill(antihisto.m_EDC_beta, beta.Mag(), p.E());
                }
            }
        }
    }

    // gamma gamma gates stuff
    for (auto &it : h.ana.gammagatedHistograms)
    {
        AgataHistograms::AgataAnaHistograms::GammaGatedHistograms &gghisto = it;

        if (gghisto.option.find("AB") == std::string::npos)
        {
            for (unsigned int a1{0}; a1 < agata.E.size(); ++a1)
            {

                double gamma1;

                // we can choose to gate either on the un or doppler corrected
                if (gghisto.option.find("DC") != std::string::npos)
                {
                    gamma1 = pDC.at(a1).E();
                }
                else
                {
                    gamma1 = agata.E.at(a1);
                }

                if (!(gamma1 > gghisto.vals.first && gamma1 < gghisto.vals.second) ||
                    (conf.agata.betaAvg.Angle(agata.Pos.at(a1)) / UNITS::deg >
                         gghisto.exclude_angle.first &&
                     conf.agata.betaAvg.Angle(agata.Pos.at(a1)) / UNITS::deg <
                         gghisto.exclude_angle.second))
                    continue;

                for (unsigned int a2{0}; a2 < agata.E.size(); a2++)
                {
                    if (a2 == a1)
                        continue;

                    Fill(gghisto.h_E, agata.E.at(a2));
                    Fill(gghisto.h_EDC_avg, pDC.at(a2).E());

                    if (gghisto.option.find("POL") != std::string::npos)
                    {
                        Fill(gghisto.m_Theta_E_avg,
                             agata.Pos.at(a1).Angle(agata.Pos.at(a2)) / UNITS::deg, agata.E.at(a2));
                        Fill(gghisto.m_Theta_EDC_avg,
                             agata.Pos.at(a1).Angle(agata.Pos.at(a2)) / UNITS::deg, pDC.at(a2).E());
                    }
                    else
                    {
                        Fill(gghisto.m_Theta_E_avg,
                             conf.agata.betaAvg.Angle(agata.Pos.at(a2)) / UNITS::deg,
                             agata.E.at(a2));
                        Fill(gghisto.m_Theta_EDC_avg,
                             conf.agata.betaAvg.Angle(agata.Pos.at(a2)) / UNITS::deg,
                             pDC.at(a2).E());
                    }
                    for (unsigned int a3{0}; a3 < agata.E.size(); ++a3)
                    {
                        if (a3 == a2 || a3 == a1)
                            continue;
                        Fill(gghisto.m_E_E, agata.E.at(a2), agata.E.at(a3));
                        Fill(gghisto.m_E_EDC_avg, agata.E.at(a2), pDC.at(a3).E());
                        Fill(gghisto.m_EDC_EDC_avg, pDC.at(a2).E(), pDC.at(a3).E());
                    }
                }
            }
        }
        else
        {

            if (settings.verbosity_level > 1)
            {
                std::cout << "Gamma Gate with AddE" << std::endl;
            }
            for (unsigned int a1{0}; a1 < **agataReader.nbAdd; ++a1)
            {

                double gamma1;
                gamma1 = agataReader.AddE->At(a1);

                if (!(gamma1 > gghisto.vals.first && gamma1 < gghisto.vals.second))
                    continue;
                if (settings.verbosity_level > 1)
                {
                    std::cout << "Gamma inside the GAMMA gate" << std::endl;
                }
                Fill(gghisto.h_AddE_mult, **agataReader.nbAdd);

                for (unsigned int a2{0}; a2 < **agataReader.nbAdd; a2++)
                {
                    if (a2 == a1)
                        continue;

                    for (unsigned int a3{0}; a3 < **agataReader.nbAdd; a3++)
                    {
                        if (a3 == a2 || a3 == a1)
                            continue;
                        Fill(gghisto.m_AddE_AddE, agataReader.AddE->At(a2),
                             agataReader.AddE->At(a3));
                        Fill(gghisto.m_AddE_AddE, agataReader.AddE->At(a3),
                             agataReader.AddE->At(a2));
                    }

                    if (**agataReader.nbAdd == 2)
                    {
                        if (a2 == a1)
                            continue;
                        Fill(gghisto.m_AddE_AddE_mult2, agataReader.AddE->At(a2),
                             agataReader.AddE->At(a1));
                        Fill(gghisto.m_AddE_AddE_mult2, agataReader.AddE->At(a1),
                             agataReader.AddE->At(a2));
                    }

                    if (**agataReader.nbAdd == 3)
                    {
                        for (unsigned int a3{0}; a3 < **agataReader.nbAdd; a3++)
                        {
                            if (a3 == a2 || a3 == a1)
                                continue;
                            Fill(gghisto.m_AddE_AddE_mult3, agataReader.AddE->At(a2),
                                 agataReader.AddE->At(a3));
                            Fill(gghisto.m_AddE_AddE_mult3, agataReader.AddE->At(a3),
                                 agataReader.AddE->At(a2));
                        }
                    }

                    if (**agataReader.nbAdd == 4)
                    {
                        for (unsigned int a3{0}; a3 < **agataReader.nbAdd; a3++)
                        {
                            if (a3 == a2 || a3 == a1)
                                continue;
                            Fill(gghisto.m_AddE_AddE_mult4, agataReader.AddE->At(a2),
                                 agataReader.AddE->At(a3));
                            Fill(gghisto.m_AddE_AddE_mult4, agataReader.AddE->At(a3),
                                 agataReader.AddE->At(a2));
                        }
                    }
                }
            }
        }
    }

    return true;
}

void AgataAnalysis::Instantiate()
{
    DetectorAnalysis::Instantiate();
    TTreeReader &fReader{input.fReader};

    if (conf.detectorsPresent.Agata)
    {
        if (input.treeToRead->FindLeaf("nbHits") == nullptr)
            throw std::runtime_error("AGATA enabled in the config but not found in the tree");
        agataReader.nbHits       = new TTreeReaderValue<Int_t>(fReader, "nbHits");
        agataReader.hitE         = new TTreeReaderArray<Float_t>(fReader, "hitE");
        agataReader.hitX         = new TTreeReaderArray<Float_t>(fReader, "hitX");
        agataReader.hitY         = new TTreeReaderArray<Float_t>(fReader, "hitY");
        agataReader.hitZ         = new TTreeReaderArray<Float_t>(fReader, "hitZ");
        agataReader.hitGX        = new TTreeReaderArray<Float_t>(fReader, "hitGX");
        agataReader.hitGY        = new TTreeReaderArray<Float_t>(fReader, "hitGY");
        agataReader.hitGZ        = new TTreeReaderArray<Float_t>(fReader, "hitGZ");
        agataReader.hitId        = new TTreeReaderArray<Int_t>(fReader, "hitId");
        agataReader.hitSg        = new TTreeReaderArray<Int_t>(fReader, "hitSg");
        agataReader.nbCores      = new TTreeReaderValue<Int_t>(fReader, "nbCores");
        agataReader.nbHitsperCry = new TTreeReaderArray<Int_t>(fReader, "nbHitsperCry");
        agataReader.coreId       = new TTreeReaderArray<Int_t>(fReader, "coreId");
        agataReader.coreE0       = new TTreeReaderArray<Float_t>(fReader, "coreE0");
        agataReader.coreE1       = new TTreeReaderArray<Float_t>(fReader, "coreE1");
        agataReader.coreT0       = new TTreeReaderArray<Float_t>(fReader, "coreT0");

        agataReader.coreTS = new TTreeReaderArray<ULong64_t>(fReader, "coreTS");
        agataReader.nbAdd  = new TTreeReaderValue<Int_t>(fReader, "nbAdd");
        agataReader.AddId  = new TTreeReaderArray<Int_t>(fReader, "AddId");
        agataReader.AddE   = new TTreeReaderArray<Float_t>(fReader, "AddE");
        agataReader.AddX   = new TTreeReaderArray<Float_t>(fReader, "AddX");
        agataReader.AddY   = new TTreeReaderArray<Float_t>(fReader, "AddY");
        agataReader.AddZ   = new TTreeReaderArray<Float_t>(fReader, "AddZ");
        agataReader.AddTS  = new TTreeReaderArray<ULong64_t>(fReader, "AddTS");
        agataReader.TSHit  = new TTreeReaderValue<ULong64_t>(fReader, "TSHit");

        if (input.treeToRead->FindLeaf("trackE") != nullptr)
        {
            agataReader.nbTrack        = new TTreeReaderValue<Int_t>(fReader, "nbTrack");
            agataReader.trackE         = new TTreeReaderArray<Float_t>(fReader, "trackE");
            agataReader.trackX1        = new TTreeReaderArray<Float_t>(fReader, "trackX1");
            agataReader.trackY1        = new TTreeReaderArray<Float_t>(fReader, "trackY1");
            agataReader.trackZ1        = new TTreeReaderArray<Float_t>(fReader, "trackZ1");
            agataReader.trackX2        = new TTreeReaderArray<Float_t>(fReader, "trackX2");
            agataReader.trackY2        = new TTreeReaderArray<Float_t>(fReader, "trackY2");
            agataReader.trackZ2        = new TTreeReaderArray<Float_t>(fReader, "trackZ2");
            agataReader.trackType      = new TTreeReaderArray<Int_t>(fReader, "trackType");
            agataReader.trackFOM       = new TTreeReaderArray<Float_t>(fReader, "trackFOM");
            agataReader.trackT         = new TTreeReaderArray<Float_t>(fReader, "trackT");
            agataReader.trackCrystalID = new TTreeReaderArray<Int_t>(fReader, "trackCrystalID");
            agataReader.TStrack        = new TTreeReaderValue<ULong64_t>(fReader, "TStrack");
            agataReader.hitT           = new TTreeReaderArray<Float_t>(fReader, "hitT");
            agataReader.trackhitE1     = new TTreeReaderArray<Float_t>(fReader, "trackhitE1");
            agataReader.trackhitE2     = new TTreeReaderArray<Float_t>(fReader, "trackhitE2");
            agataReader.tracknbHits    = new TTreeReaderArray<Int_t>(fReader, "tracknbHits");
        }
    }
}

void AgataAnalysis::Finalize()
{
    DetectorAnalysis::Finalize();
    NormalizeHistos<DetectorConf>(conf.agata);
}
