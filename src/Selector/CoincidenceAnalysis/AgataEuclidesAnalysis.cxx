#include "AgataEuclidesAnalysis.h"

AgataEuclidesAnalysis::AgataEuclidesAnalysis(
    DetectorAnalysis::Inputs& in, Agata& a, Euclides& e,
    AgataAnalysis::AgataReader& ra, EuclidesAnalysis::EuclidesReader& re,
    GammaDC& gdc)
    : GammadetectorEuclidesAnalysis(in,
                                    ConfManager::getInstance().agataEuclides,
                                    a, ConfManager::getInstance().agata, e, ra,
                                    re, gdc, agataEuclidesHistograms),
      agata(a), euclides(e), agataReader(ra) {
    if(coincidenceConf.enable_treeFill && input.type == Inputs::Type::SIGNAL && input.tree)
        input.tree->Branch("AgataEuclidesDC", &gdc);
    
    //Silence warnings
    (void)agata;    
    (void)euclides;    
    (void)agataReader;
}

bool AgataEuclidesAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    AgataEuclidesHistograms& h = agataEuclidesHistograms;
    h.dir
        = input.outFile.mkdir("AgataEuclides", "Agata-Euclides coincidences");
    h.dir->cd();

    GammadetectorEuclidesAnalysis::AllocateHistos();
    return true;
}

void AgataEuclidesAnalysis::Analyze() {
    GammadetectorEuclidesAnalysis::Analyze();
}

bool AgataEuclidesAnalysis::FillHistos() {
    if(!GammadetectorEuclidesAnalysis::FillHistos()) return false;

    return true;
}

void AgataEuclidesAnalysis::Finalize() {
    GammadetectorEuclidesAnalysis::Finalize();
}
