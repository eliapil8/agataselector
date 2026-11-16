#include "LabrEXSSiDeAnalysis.h"

LabrEXSSiDeAnalysis::LabrEXSSiDeAnalysis(DetectorAnalysis::Inputs& in, Labr& a,
                                       EXSSiDe& s, 
                                       LabrAnalysis::LabrReader&     rl,
                                       EXSSiDeAnalysis::EXSSiDeReader& rs,
                                       std::vector<GammaDC>&         gdc)
    : GammadetectorEXSSiDeAnalysis(in, ConfManager::getInstance().labrEXSSiDe, a, 
                                ConfManager::getInstance().labr, s,
                                rl, rs, gdc, labrEXSSiDeHistograms),
      labr(a), exsside(s), labrReader(rl) {
        (void)labrReader;
        (void)exsside;
        (void)labr;
    }


bool LabrEXSSiDeAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    GammadetectorEXSSiDeAnalysis::AllocateHistos();
    return true;
}


void LabrEXSSiDeAnalysis::Analyze() { GammadetectorEXSSiDeAnalysis::Analyze(); }

void LabrEXSSiDeAnalysis::Finalize() {
    GammadetectorEXSSiDeAnalysis::Finalize();
}
