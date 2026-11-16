#include "LabrSauronAnalysis.h"

LabrSauronAnalysis::LabrSauronAnalysis(DetectorAnalysis::Inputs& in, Labr& a,
                                       Sauron& s, SauronConf& sc,
                                       LabrSauronConf&               c,
                                       LabrAnalysis::LabrReader&     rl,
                                       SauronAnalysis::SauronReader& rs,
                                       std::vector<GammaDC>&         gdc)
    : GammadetectorSauronAnalysis(in, c, a, ConfManager::getInstance().labr, s,
                                  sc, rl, rs, gdc, labrSauronHistograms),
      labr(a), sauron(s), labrSauronConf(c), labrReader(rl) {

        (void)labr;
        (void)sauron;
        (void)labrSauronConf;
        (void)labrReader;
    }

LabrSauronFwAnalysis::LabrSauronFwAnalysis(DetectorAnalysis::Inputs& in,
                                           Labr& a, Sauron& s,
                                           LabrAnalysis::LabrReader&     rl,
                                           SauronAnalysis::SauronReader& rs,
                                           std::vector<GammaDC>&         gdc)
    : LabrSauronAnalysis(in, a, s, ConfManager::getInstance().sauronFw,
                         ConfManager::getInstance().labrSauronFw, rl, rs,
                         gdc) {
    if(coincidenceConf.enable_treeFill && input.type == Inputs::Type::SIGNAL && input.tree)
        input.tree->Branch("LabrSauronFwDC", &gdc);
}

LabrSauronBwAnalysis::LabrSauronBwAnalysis(DetectorAnalysis::Inputs& in,
                                           Labr& a, Sauron& s,
                                           LabrAnalysis::LabrReader&     rl,
                                           SauronAnalysis::SauronReader& rs,
                                           std::vector<GammaDC>&         gdc)
    : LabrSauronAnalysis(in, a, s, ConfManager::getInstance().sauronBw,
                         ConfManager::getInstance().labrSauronBw, rl, rs,
                         gdc) {
    if(coincidenceConf.enable_treeFill)
        input.tree->Branch("LabrSauronBwDC", &gdc);
}

bool LabrSauronAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    GammadetectorSauronAnalysis::AllocateHistos();
    return true;
}

bool LabrSauronFwAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    LabrSauronHistograms& h = labrSauronHistograms;
    h.dir = input.outFile.mkdir("LabrSauronFw", "Labr-SauronFw coincidences");
    h.dir->cd();
    LabrSauronAnalysis::AllocateHistos();
    return true;
}

bool LabrSauronBwAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    LabrSauronHistograms& h = labrSauronHistograms;
    h.dir = input.outFile.mkdir("LabrSauronBw", "Labr-SauronBw coincidences");
    h.dir->cd();
    LabrSauronAnalysis::AllocateHistos();
    return true;
}

void LabrSauronAnalysis::Analyze() { GammadetectorSauronAnalysis::Analyze(); }

void LabrSauronAnalysis::Finalize() {
    GammadetectorSauronAnalysis::Finalize();
}
