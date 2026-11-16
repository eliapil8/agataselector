#include "AgataSpiderAnalysis.h"

AgataSpiderAnalysis::AgataSpiderAnalysis(DetectorAnalysis::Inputs& in,
                                         Agata& a, Spider& s,
                                         AgataAnalysis::AgataReader&   ra,
                                         SpiderAnalysis::SpiderReader& rs,
                                         std::vector<GammaDC>&         gdc)
    : GammadetectorSpiderAnalysis(in, ConfManager::getInstance().agataSpider,
                                  a, ConfManager::getInstance().agata, s, ra,
                                  rs, gdc, agataSpiderHistograms),
      agata(a), spider(s), agataReader(ra) {
    if(coincidenceConf.enable_treeFill && input.type == Inputs::Type::SIGNAL && input.tree)
        input.tree->Branch("AgataSpiderDC", &gdc);

    //Silence warnings
    (void)agata;    
    (void)spider;    
    (void)agataReader;
}

bool AgataSpiderAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    AgataSpiderHistograms& h = agataSpiderHistograms;
    h.dir = input.outFile.mkdir("AgataSpider", "Agata-Spider coincidences");
    h.dir->cd();
    GammadetectorSpiderAnalysis::AllocateHistos();
    return true;
}

void AgataSpiderAnalysis::Analyze() { 
    GammadetectorSpiderAnalysis::Analyze(); 
}

void AgataSpiderAnalysis::Finalize() {
    GammadetectorSpiderAnalysis::Finalize();
}
