#include "GammadetectorDanteAnalysis.h"

GammadetectorDanteAnalysis::GammadetectorDanteAnalysis(
    Inputs& in, GammadetectorDanteConf& gpConf, Gammadetector& g,
    GammadetectorConf& gConf, Dante& d, Readers& rg,
    DanteAnalysis::DanteReader& rd, std::vector<GammaDC>& gdc,
    GammadetectorDanteHistograms& gdh)
    : CoincidenceAnalysis(in, g, gConf, d, ConfManager::getInstance().dante,
                          gpConf, rg, rd, gdh),
      gammadetector(g), dante(d), gammadetectorDanteConf(gpConf),
      gammadetectorConf(gConf), gammaDC(gdc),
      gammadetectorDanteHistograms(gdh), danteReader(rd) {

        (void)danteReader;
    }

bool GammadetectorDanteAnalysis::AllocateHistos() {
    if(!CoincidenceAnalysis::AllocateHistos()) return false;

    GammadetectorDanteHistograms& h = gammadetectorDanteHistograms;
    h.dir->cd();

    Initialize<TH2D>(h.m_TSdiff_TS,
                     new TH2D("m_TSdiff_TS", "m_TSdiff_TS", 12 * 360, 0,
                              12 * 3600,
                              (gammadetectorDanteConf.windowRight
                               - gammadetectorDanteConf.windowLeft + 200),
                              gammadetectorDanteConf.windowLeft - 100,
                              gammadetectorDanteConf.windowRight + 100),
                     h.ptrs);

    Initialize<TH2D>(
        h.m_TSdantemTSgamma_danteID,
        new TH2D("m_TSdantemTSgamma_danteID",
                 "DanteTS - GammaTS vs Dante ID; Dante ID; TS difference [10 "
                 "ns]; Counts",
                 10, 0, 10,
                 (gammadetectorDanteConf.windowRight
                  - gammadetectorDanteConf.windowLeft + 200)
                     * 10,
                 gammadetectorDanteConf.windowLeft - 100,
                 gammadetectorDanteConf.windowRight + 100),
        h.ptrs);

    Initialize<TH1D>(h.h_cTime_gated,
                     new TH1D("h_cTime_gated", "h_cTime_gated",
                              (gammadetectorDanteConf.windowRight
                               - gammadetectorDanteConf.windowLeft + 200)
                                  * 10,
                              gammadetectorDanteConf.windowLeft - 100,
                              gammadetectorDanteConf.windowRight + 100),
                     h.ptrs);

    Initialize<TH1D>(
        h.h_E,
        new TH1D(
            "h_E",
            Form("Dante coincidence Gamma energy;Energy [keV];Counts/%d keV",
                 gammadetectorConf.binWidth),
            static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                             / gammadetectorConf.binWidth),
            0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH1D>(
        h.h_EDCBP,
        new TH1D("h_EDCBP",
                 Form("Doppler corrected gamma energy for binary "
                      "partner;Energy [keV];Counts/%d keV",
                      gammadetectorConf.binWidth),
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);
    Initialize<TH2D>(h.h_p4_theta_phi,
                     new TH2D("h_p4_theta_phi",
                              "p4 angle ; theta ; phi; counts ", 180, 0, 180,
                              360, -180, 180),
                     h.ptrs);
    Initialize<TH1D>(
        h.h_beta_BP,
        new TH1D("h_beta_BP",
                 "velocity for ion bp detected by DANTE; beta [*100]; counts ",
                 150, 0, 15),
        h.ptrs);
    return true;
}

void GammadetectorDanteAnalysis::Analyze() {
    CoincidenceAnalysis::Analyze();
    gammaDC.clear();
    //if(!in_coinc) return;
    gammaDC.resize(coincMatrix.size());

    for(size_t v{0}; v<coincMatrix.size(); v++){
        
        auto &currentIon = gammaDC.at(v);
        currentIon.resize(gammadetector.P.size());

        for(size_t s{0}; s<currentIon.E.size(); s++){

            gammaDC.at(v).p4
                = SlowInTarget(dante.p4.at(v), gammadetectorDanteConf.bpDcPos,
                               conf.dante.ion);
            
            if(coincMatrix.at(v).at(s)){
                currentIon.inCoinc.at(s) = true;
            } else {
                currentIon.inCoinc.at(s) = false;
            }

            currentIon.E.at(s)     = gammadetector.P.at(s);
            currentIon.EDC.at(s)   = gammadetector.P.at(s);
            currentIon.EDCBP.at(s) = gammadetector.P.at(s);

            // TODO: what about p3?
            currentIon.EDCBP.at(s).Boost(-currentIon.p4.BoostVector());

        }
    }
}

bool GammadetectorDanteAnalysis::FillHistos() {
    if(!CoincidenceAnalysis::FillHistos()) return false;

    GammadetectorDanteHistograms& h = gammadetectorDanteHistograms;

    for(unsigned int s{0}; s < dante.Pos.size(); ++s) {
        for(unsigned int a{0}; a < gammadetector.E.size(); ++a) {
            if(!gammaDC.at(s).inCoinc.at(a)) continue;
            auto diff = unsignedDiff<double, unsigned long long>(
                            dante.TS, gammadetector.TS)
                      + (dante.Time.at(s) - gammadetector.Time.at(a))
                            / (10. * UNITS::ns);
            Fill(h.h_cTime_gated, diff);

            Fill(h.h_E, gammadetector.E.at(a));
            Fill(h.h_EDCBP, gammaDC.at(s).EDC.at(a).E());

            Fill(h.h_beta_BP, gammaDC.at(s).p4.Beta() * 100);

            Fill(h.h_p4_theta_phi, gammaDC.at(s).p4.Theta() / UNITS::deg,
                 gammaDC.at(s).p4.Phi() / UNITS::deg);
        }
    }
    return true;
}

void GammadetectorDanteAnalysis::Finalize() {
    CoincidenceAnalysis::Finalize();
}
void GammadetectorDanteAnalysis::Clear() { gammaDC.clear(); }
