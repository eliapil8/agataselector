#include "AgataEXSSiDeAnalysis.h"

AgataEXSSiDeAnalysis::AgataEXSSiDeAnalysis(DetectorAnalysis::Inputs& in,
                                         Agata& a, EXSSiDe& s, 
                                         AgataAnalysis::AgataReader&   ra,
                                         EXSSiDeAnalysis::EXSSiDeReader& rs,
                                         std::vector<GammaDC>&         gdc)
    : GammadetectorEXSSiDeAnalysis(in, ConfManager::getInstance().agataEXSSiDe, a, 
                                    ConfManager::getInstance().agata,
                                  s, ra, rs, gdc, agataEXSSiDeHistograms),
      agata(a), exsside(s),  agataReader(ra) {
    
    if(coincidenceConf.enable_treeFill && input.type == Inputs::Type::SIGNAL && input.tree)
        input.tree->Branch("AgataExSiDeDC", &gdc);
    
    //Silence warnings
    (void)agata;    
    (void)exsside;    
    (void)agataReader;
}


bool AgataEXSSiDeAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    if(!GammadetectorEXSSiDeAnalysis::AllocateHistos()) return false;
    return true;
}

void AgataEXSSiDeAnalysis::Analyze() { GammadetectorEXSSiDeAnalysis::Analyze(); }

void AgataEXSSiDeAnalysis::Finalize() {
    GammadetectorEXSSiDeAnalysis::Finalize();
}
