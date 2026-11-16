#include "LabrDanteAnalysis.h"

LabrDanteAnalysis::LabrDanteAnalysis(DetectorAnalysis::Inputs& in, Labr& l,
                                     Dante& d, LabrAnalysis::LabrReader& rl,
                                     DanteAnalysis::DanteReader& rd,
                                     std::vector<GammaDC>&       gdc)
    : GammadetectorDanteAnalysis(in, ConfManager::getInstance().labrDante, l,
                                 ConfManager::getInstance().labr, d, rl, rd,
                                 gdc, labrDanteHistograms),
      labr(l), dante(d), labrReader(rl) {
    if(coincidenceConf.enable_treeFill && input.type == Inputs::Type::SIGNAL && input.tree)
        input.tree->Branch("LabrDanteDC", &gdc);
    
    //Silence warnings
    (void)labr;    
    (void)dante;    
    (void)labrReader;
}

bool LabrDanteAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    LabrDanteHistograms& h = labrDanteHistograms;
    h.dir = input.outFile.mkdir("LabrDante", "Labr-Dante coincidences");
    h.dir->cd();
    GammadetectorDanteAnalysis::AllocateHistos();

    Initialize<TH2D>(
        h.m_TSdantemTSgamma_labrID,
        new TH2D("m_TSdantemTSgamma_labrID",
                 "DanteTS - GammaTS vs LaBr ID; LaBr ID; TS "
                 "difference [10 ns]; Counts",
                 64, 0, 64,
                 (ConfManager::getInstance().labrDante.windowRight
                  - ConfManager::getInstance().labrDante.windowLeft + 200)
                     * 10,
                 ConfManager::getInstance().labrDante.windowLeft - 100,
                 ConfManager::getInstance().labrDante.windowRight + 100),
        h.ptrs);
    return true;
}

void LabrDanteAnalysis::Analyze() { GammadetectorDanteAnalysis::Analyze(); }

void LabrDanteAnalysis::Finalize() { GammadetectorDanteAnalysis::Finalize(); }
