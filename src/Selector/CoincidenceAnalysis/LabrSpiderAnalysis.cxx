#include "LabrSpiderAnalysis.h"

LabrSpiderAnalysis::LabrSpiderAnalysis(DetectorAnalysis::Inputs& in, Labr& l,
                                       Spider& p, LabrAnalysis::LabrReader& rl,
                                       SpiderAnalysis::SpiderReader& rd,
                                       std::vector<GammaDC>&         gdc)
    : GammadetectorSpiderAnalysis(in, ConfManager::getInstance().labrSpider, l,
                                  ConfManager::getInstance().labr, p, rl, rd,
                                  gdc, labrSpiderHistograms),
      labr(l), spider(p), labrReader(rl) {
    if(coincidenceConf.enable_treeFill && input.type == Inputs::Type::SIGNAL && input.tree)
        input.tree->Branch("LabrSpiderDC", &gdc);

    //Silence warnings
    (void)labr;    
    (void)spider;    
    (void)labrReader;
}

bool LabrSpiderAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    LabrSpiderHistograms& h = labrSpiderHistograms;
    h.dir = input.outFile.mkdir("LabrSpider", "Labr-Spider coincidences");
    h.dir->cd();
    GammadetectorSpiderAnalysis::AllocateHistos();

    Initialize<TH2D>(
        h.m_TSspidermTSgamma_labrID,
        new TH2D("m_TSspidermTSgamma_labrID",
                 "SpiderTS - GammaTS vs LaBr ID; LaBr ID; TS "
                 "difference [10 ns]; Counts",
                 64, 0, 64,
                 (ConfManager::getInstance().labrSpider.windowRight
                  - ConfManager::getInstance().labrSpider.windowLeft + 200)
                     * 10,
                 ConfManager::getInstance().labrSpider.windowLeft - 100,
                 ConfManager::getInstance().labrSpider.windowRight + 100),
        h.ptrs);
    return true;
}

void LabrSpiderAnalysis::Analyze() { GammadetectorSpiderAnalysis::Analyze(); }

void LabrSpiderAnalysis::Finalize() {
    GammadetectorSpiderAnalysis::Finalize();
}
