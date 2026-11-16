#include "AgataLabrAnalysis.h"

AgataLabrAnalysis::AgataLabrAnalysis(Inputs& in, Agata& a, Labr& l,
                                     AgataAnalysis::AgataReader& ra,
                                     LabrAnalysis::LabrReader&   rl)
    : CoincidenceAnalysis(in, a, ConfManager::getInstance().agata, l,
                          ConfManager::getInstance().labr,
                          ConfManager::getInstance().agataLabr, rl, ra,
                          agataLabrHistograms),
      agata(a), labr(l), agataReader(ra), labrReader(rl) {
    
    //Silence warnings
    (void)agata;
    (void)labr;
    (void)agataReader;
    (void)labrReader;
}

bool AgataLabrAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;

    AgataLabrHistograms& h = agataLabrHistograms;
    h.dir = input.outFile.mkdir("AgataLabr", "Agata-LaBr coincidences");
    h.dir->cd();
    CoincidenceAnalysis::AllocateHistos();

    Initialize<TH2D>(
        h.m_TSdiff_TS,
        new TH2D(
            "m_TSdiff_TS", "m_TSdiff_TS", 12 * 360, 0, 12 * 3600,
            (conf.agataLabr.windowRight - conf.agataLabr.windowLeft + 200),
            conf.agataLabr.windowLeft - 100, conf.agataLabr.windowRight + 100),
        h.ptrs);

    Initialize<TH2D>(
        h.m_TSlabrmTSagata_labrID,
        new TH2D("m_TSlabrmTSagata_labrID",
                 "LaBrTS - AgataTS vs LaBr ID; LaBr ID; TS difference [10 "
                 "ns]; Counts",
                 64, 0, 64,
                 (conf.agataLabr.windowRight - conf.agataLabr.windowLeft + 200)
                     * 10,
                 conf.agataLabr.windowLeft - 100,
                 conf.agataLabr.windowRight + 100),
        h.ptrs);

    Initialize<TH1D>(
        h.h_cTime_gated,
        new TH1D("h_cTime_gated", "h_cTime_gated",
                 (conf.agataLabr.windowRight - conf.agataLabr.windowLeft + 200)
                     * 10,
                 conf.agataLabr.windowLeft - 100,
                 conf.agataLabr.windowRight + 100),
        h.ptrs);

    Initialize<TH1D>(
        h.h_Eaga,
        new TH1D("h_Eaga",
                 Form("AGATA Gamma energy;Energy [keV];Counts/%d keV",
                      conf.agata.binWidth),
                 static_cast<int>(conf.agata.gammaEmax / UNITS::keV
                                  / conf.agata.binWidth),
                 0, conf.agata.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH1D>(
        h.h_Elabr,
        new TH1D("h_Elabr",
                 Form("LaBr Gamma energy;Energy [keV];Counts/%d keV",
                      conf.labr.binWidth),
                 static_cast<int>(conf.labr.gammaEmax / UNITS::keV
                                  / conf.labr.binWidth),
                 0, conf.labr.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_Eaga_Elabr,
        new TH2D("m_Eaga_Elabr",
                 "AGATA - LaBr Gamma-gamma matrix;AGATA Energy [keV];LaBr "
                 "Energy [keV];Counts",
                 static_cast<int>(conf.agata.gammaEmax / UNITS::keV
                                  / conf.agata.binWidth),
                 0, conf.agata.gammaEmax / UNITS::keV,
                 static_cast<int>(conf.labr.gammaEmax / UNITS::keV
                                  / conf.labr.binWidth),
                 0, conf.labr.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_EE_labr,
        new TH2D(
            "m_Labr_E_E",
            "LaBr - LaBr Gamma-gamma matrix;Energy [keV];Energy [keV];Counts",
            static_cast<int>(conf.labr.gammaEmax / UNITS::keV
                             / conf.labr.binWidth),
            0, conf.labr.gammaEmax / UNITS::keV,
            static_cast<int>(conf.labr.gammaEmax / UNITS::keV
                             / conf.labr.binWidth),
            0, conf.labr.gammaEmax / UNITS::keV),
        h.ptrs);
    Initialize<TH2D>(
        h.m_Eaga_LabrTdiff,
        new TH2D("m_Eaga_LabrTdiff",
                 "AGATA energy vs LaBr-LaBr time difference matrix;AGATA "
                 "Energy [keV];LaBr-Labr Time Difference [ns];Counts",
                 static_cast<int>(conf.agata.gammaEmax / UNITS::keV
                                  / conf.agata.binWidth),
                 0, conf.agata.gammaEmax / UNITS::keV, 4000, 0, 400),
        h.ptrs);
    return true;
}

void AgataLabrAnalysis::Analyze() {
    CoincidenceAnalysis::Analyze();
   
}

bool AgataLabrAnalysis::FillHistos() {
    if(!CoincidenceAnalysis::FillHistos()) return false;

    AgataLabrHistograms& h = agataLabrHistograms;

    for(int a{0}; a < static_cast<int>(agata.E.size()); ++a) {
        Fill(h.h_Eaga, agata.E.at(a));
        for(int l1{0}; l1 < static_cast<int>(labr.E.size()); ++l1) {
            if(!coincMatrix.at(l1).at(a)) continue;
            auto diff
                = unsignedDiff<double, unsigned long long>(labr.TS, agata.TS)
                + (labr.Time.at(l1) - agata.Time.at(a)) / (10. * UNITS::ns);
            Fill(h.h_cTime_gated, diff);
            Fill(h.m_Eaga_Elabr, agata.E.at(a), labr.E.at(l1));

            for(int l2{0}; l2 < l1; ++l2) {
                if(!coincMatrix.at(l2).at(a)) continue;

                Fill(h.m_EE_labr, labr.E[l1], labr.E[l2]);
                Fill(h.m_EE_labr, labr.E[l2], labr.E[l1]);

                Fill(h.m_Eaga_LabrTdiff, agata.E.at(a),
                     abs(labr.Time[l1] - labr.Time[l2]) / UNITS::ns);
            }
        }
    }

    for(int l{0}; l < static_cast<int>(labr.E.size()); ++l) {
        Fill(h.h_Elabr, labr.E.at(l));
    }

    return true;
}

void AgataLabrAnalysis::Finalize() { CoincidenceAnalysis::Finalize(); }
