#include "LabrEuclidesAnalysis.h"

LabrEuclidesAnalysis::LabrEuclidesAnalysis(
    DetectorAnalysis::Inputs& in, Labr& l, Euclides& e,
    LabrAnalysis::LabrReader& rl, EuclidesAnalysis::EuclidesReader& re,
    GammaDC& gdc)
    : GammadetectorEuclidesAnalysis(in,
                                    ConfManager::getInstance().labrEuclides, l,
                                    ConfManager::getInstance().labr, e, rl, re,
                                    gdc, labrEuclidesHistograms),
      labr(l), euclides(e), labrReader(rl) {
    if(coincidenceConf.enable_treeFill && input.type == Inputs::Type::SIGNAL && input.tree)
        input.tree->Branch("LabrEuclidesDC", &gdc);

    //Silence warnings
    (void)labr;    
    (void)euclides;    
    (void)labrReader;

}

bool LabrEuclidesAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    LabrEuclidesHistograms& h = labrEuclidesHistograms;
    h.dir = input.outFile.mkdir("LabrEuclides", "Labr-Euclides coincidences");
    h.dir->cd();
    GammadetectorEuclidesAnalysis::AllocateHistos();

    Initialize<TH2D>(
        h.m_TSeuclidesmTSgamma_labrID,
        new TH2D("m_TSeuclidesmTSgamma_labrID",
                 "EuclidesTS - GammaTS vs LaBr ID; LaBr ID; TS "
                 "difference [10 ns]; Counts",
                 64, 0, 64,
                 (ConfManager::getInstance().labrEuclides.windowRight
                  - ConfManager::getInstance().labrEuclides.windowLeft + 200)
                     * 10,
                 ConfManager::getInstance().labrEuclides.windowLeft - 100,
                 ConfManager::getInstance().labrEuclides.windowRight + 100),
        h.ptrs);
    return true;
}

void LabrEuclidesAnalysis::Analyze() {
    GammadetectorEuclidesAnalysis::Analyze();
}

void LabrEuclidesAnalysis::Finalize() {
    GammadetectorEuclidesAnalysis::Finalize();
}
