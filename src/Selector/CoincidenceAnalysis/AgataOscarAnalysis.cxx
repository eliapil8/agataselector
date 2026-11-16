#include "AgataOscarAnalysis.h"

AgataOscarAnalysis::AgataOscarAnalysis(DetectorAnalysis::Inputs& in, Agata& a,
                                       Oscar& s, OscarConf& sc,
                                       AgataOscarConf&             c,
                                       AgataAnalysis::AgataReader& ra,
                                       OscarAnalysis::OscarReader& ro,
                                       std::vector<GammaDC>&       gdc)
    : GammadetectorOscarAnalysis(in, c, a, ConfManager::getInstance().agata, s,
                                 sc, ra, ro, gdc, agataOscarHistograms),
      agata(a), oscar(s), agataOscarConf(c), agataReader(ra) {
        (void)agata;
        (void)oscar;
        (void)agataOscarConf;
        (void)agataReader;
}


AgataOscarUpAnalysis::AgataOscarUpAnalysis(DetectorAnalysis::Inputs& in,
                                           Agata& a, Oscar& s,
                                           AgataAnalysis::AgataReader& ra,
                                           OscarAnalysis::OscarReader& ro,
                                           std::vector<GammaDC>&       gdc)
    : AgataOscarAnalysis(in, a, s, ConfManager::getInstance().oscarUp,
                         ConfManager::getInstance().agataOscarUp, ra, ro,
                         gdc) {
    if(coincidenceConf.enable_treeFill && input.type == Inputs::Type::SIGNAL && input.tree)
        input.tree->Branch("AgataOscarUpDC", &gdc);
}

AgataOscarDownAnalysis::AgataOscarDownAnalysis(DetectorAnalysis::Inputs& in,
                                               Agata& a, Oscar& s,
                                               AgataAnalysis::AgataReader& ra,
                                               OscarAnalysis::OscarReader& ro,
                                               std::vector<GammaDC>&       gdc)
    : AgataOscarAnalysis(in, a, s, ConfManager::getInstance().oscarDown,
                         ConfManager::getInstance().agataOscarDown, ra, ro,
                         gdc) {
    if(coincidenceConf.enable_treeFill)
        input.tree->Branch("AgataOscarDownDC", &gdc);
}

bool AgataOscarAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    if(!GammadetectorOscarAnalysis::AllocateHistos()) return false;
    return true;
}

bool AgataOscarUpAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    AgataOscarHistograms& h = agataOscarHistograms;
    h.dir = input.outFile.mkdir("AgataOscarUp", "Agata-OscarUp coincidences");
    h.dir->cd();
    if(AgataOscarAnalysis::AllocateHistos()) return false;
    return true;
}

bool AgataOscarDownAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    AgataOscarHistograms& h = agataOscarHistograms;
    h.dir                   = input.outFile.mkdir("AgataOscarDown",
                                                  "Agata-OscarDown coincidences");
    h.dir->cd();
    if(!AgataOscarAnalysis::AllocateHistos()) return false;
    return true;
}

void AgataOscarAnalysis::Analyze() { GammadetectorOscarAnalysis::Analyze(); }

void AgataOscarAnalysis::Finalize() { GammadetectorOscarAnalysis::Finalize(); }
