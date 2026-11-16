#include "LabrOscarAnalysis.h"

LabrOscarAnalysis::LabrOscarAnalysis(DetectorAnalysis::Inputs& in, Labr& a,
                                     Oscar& s, OscarConf& sc, LabrOscarConf& c,
                                     LabrAnalysis::LabrReader&   rl,
                                     OscarAnalysis::OscarReader& ro,
                                     std::vector<GammaDC>&       gdc)
    : GammadetectorOscarAnalysis(in, c, a, ConfManager::getInstance().labr, s,
                                 sc, rl, ro, gdc, labrOscarHistograms),
      labr(a), oscar(s), labrOscarConf(c), labrReader(rl) {

        (void)labr;
        (void)oscar;
        (void)labrOscarConf;
        (void)labrReader;
    }

LabrOscarUpAnalysis::LabrOscarUpAnalysis(DetectorAnalysis::Inputs& in, Labr& a,
                                         Oscar&                      s,
                                         LabrAnalysis::LabrReader&   rl,
                                         OscarAnalysis::OscarReader& ro,
                                         std::vector<GammaDC>&       gdc)
    : LabrOscarAnalysis(in, a, s, ConfManager::getInstance().oscarUp,
                        ConfManager::getInstance().labrOscarUp, rl, ro, gdc) {
    if(coincidenceConf.enable_treeFill && input.type == Inputs::Type::SIGNAL && input.tree)
        input.tree->Branch("LabrOscarUpDC", &gdc);
}

LabrOscarDownAnalysis::LabrOscarDownAnalysis(DetectorAnalysis::Inputs& in,
                                             Labr& a, Oscar& s,
                                             LabrAnalysis::LabrReader&   rl,
                                             OscarAnalysis::OscarReader& ro,
                                             std::vector<GammaDC>&       gdc)
    : LabrOscarAnalysis(in, a, s, ConfManager::getInstance().oscarDown,
                        ConfManager::getInstance().labrOscarDown, rl, ro,
                        gdc) {
    if(coincidenceConf.enable_treeFill)
        input.tree->Branch("LabrOscarDownDC", &gdc);
}

bool LabrOscarAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    GammadetectorOscarAnalysis::AllocateHistos();
    return true;
}

bool LabrOscarUpAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    LabrOscarHistograms& h = labrOscarHistograms;
    h.dir = input.outFile.mkdir("LabrOscarUp", "Labr-OscarUp coincidences");
    h.dir->cd();
    LabrOscarAnalysis::AllocateHistos();
    return true;
}

bool LabrOscarDownAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    LabrOscarHistograms& h = labrOscarHistograms;
    h.dir
        = input.outFile.mkdir("LabrOscarDown", "Labr-OscarDown coincidences");
    h.dir->cd();
    LabrOscarAnalysis::AllocateHistos();
    return true;
}

void LabrOscarAnalysis::Analyze() { GammadetectorOscarAnalysis::Analyze(); }

void LabrOscarAnalysis::Finalize() { GammadetectorOscarAnalysis::Finalize(); }
