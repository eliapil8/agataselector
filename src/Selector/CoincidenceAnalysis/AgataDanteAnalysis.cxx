#include "AgataDanteAnalysis.h"

AgataDanteAnalysis::AgataDanteAnalysis(DetectorAnalysis::Inputs& in, Agata& a,
                                       Dante&                      d,
                                       AgataAnalysis::AgataReader& ra,
                                       DanteAnalysis::DanteReader& rd,
                                       std::vector<GammaDC>&       gdc)
    : GammadetectorDanteAnalysis(in, ConfManager::getInstance().agataDante, a,
                                 ConfManager::getInstance().agata, d, ra, rd,
                                 gdc, agataDanteHistograms),
      agata(a), dante(d), agataReader(ra) {
    if(coincidenceConf.enable_treeFill && input.type == Inputs::Type::SIGNAL && input.tree)
        input.tree->Branch("AgataDanteDC", &gdc);
    
    //Silence warnings
    (void)agata;    
    (void)dante;    
    (void)agataReader;
}

bool AgataDanteAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    AgataDanteHistograms& h = agataDanteHistograms;
    h.dir = input.outFile.mkdir("AgataDante", "Agata-Dante coincidences");
    h.dir->cd();
    GammadetectorDanteAnalysis::AllocateHistos();
    return true;
}

void AgataDanteAnalysis::Analyze() { GammadetectorDanteAnalysis::Analyze(); }

void AgataDanteAnalysis::Finalize() { GammadetectorDanteAnalysis::Finalize(); }
