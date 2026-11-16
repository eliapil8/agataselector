#include "GammadetectorEXSSiDeAnalysis.h"

GammadetectorEXSSiDeAnalysis::GammadetectorEXSSiDeAnalysis(
    Inputs& in, GammadetectorEXSSiDeConf& gpConf, Gammadetector& g,
    GammadetectorConf& gConf, EXSSiDe& s, Readers& rg,
    EXSSiDeAnalysis::EXSSiDeReader& rs, std::vector<GammaDC>& gdc,
    GammadetectorEXSSiDeHistograms& gsh)
    : CoincidenceAnalysis(in, g, gConf, s, ConfManager::getInstance().exsside,
                         gpConf, rg, rs, gsh),
      gammadetector(g), exsside(s), 
      gammadetectorEXSSiDeConf(gpConf), gammadetectorConf(gConf),
      gammaDC(gdc),
      gammadetectorEXSSiDeHistograms(gsh), exssideReader(rs) {

        (void)exssideReader;
        (void)exsside;
        (void)gammadetector;
        (void)gammadetectorEXSSiDeConf;
        (void)gammadetectorConf;
    }

bool GammadetectorEXSSiDeAnalysis::AllocateHistos() {
    GammadetectorEXSSiDeHistograms& h = gammadetectorEXSSiDeHistograms;
    h.dir->cd();

    if(!CoincidenceAnalysis::AllocateHistos()) return false;

    return true;
}

void GammadetectorEXSSiDeAnalysis::Clear() { gammaDC.clear(); }

void GammadetectorEXSSiDeAnalysis::Analyze() {
    CoincidenceAnalysis::Analyze();
    //if(!in_coinc) return;
    gammaDC.clear();
}

bool GammadetectorEXSSiDeAnalysis::FillHistos() {
    if(!CoincidenceAnalysis::FillHistos()) return false;
    //if(!in_coinc) return true;
    return true;
}

void GammadetectorEXSSiDeAnalysis::Finalize() {
    CoincidenceAnalysis::Finalize();
}
