#include "AgataSauronAnalysis.h"

AgataSauronAnalysis::AgataSauronAnalysis(DetectorAnalysis::Inputs& in,
                                         Agata& a, Sauron& s, SauronConf& sc,
                                         AgataSauronConf&              c,
                                         AgataAnalysis::AgataReader&   ra,
                                         SauronAnalysis::SauronReader& rs,
                                         std::vector<GammaDC>&         gdc)
    : GammadetectorSauronAnalysis(in, c, a, ConfManager::getInstance().agata,
                                  s, sc, ra, rs, gdc, agataSauronHistograms),
      agata(a), sauron(s), agataSauronConf(c), agataReader(ra) {
        (void)agata;
        (void)sauron;
        (void)agataSauronConf;
        (void)agataReader;
    }

AgataSauronFwAnalysis::AgataSauronFwAnalysis(DetectorAnalysis::Inputs& in,
                                             Agata& a, Sauron& s,
                                             AgataAnalysis::AgataReader&   ra,
                                             SauronAnalysis::SauronReader& rs,
                                             std::vector<GammaDC>&         gdc)
    : AgataSauronAnalysis(in, a, s, ConfManager::getInstance().sauronFw,
                          ConfManager::getInstance().agataSauronFw, ra, rs,
                          gdc) {
    if(coincidenceConf.enable_treeFill && input.type == Inputs::Type::SIGNAL && input.tree)
        input.tree->Branch("AgataSauronFwDC", &gdc);
}

AgataSauronBwAnalysis::AgataSauronBwAnalysis(DetectorAnalysis::Inputs& in,
                                             Agata& a, Sauron& s,
                                             AgataAnalysis::AgataReader&   ra,
                                             SauronAnalysis::SauronReader& rs,
                                             std::vector<GammaDC>&         gdc)
    : AgataSauronAnalysis(in, a, s, ConfManager::getInstance().sauronBw,
                          ConfManager::getInstance().agataSauronBw, ra, rs,
                          gdc) {
    if(coincidenceConf.enable_treeFill)
        input.tree->Branch("AgataSauronBwDC", &gdc);
}

bool AgataSauronAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    if(!GammadetectorSauronAnalysis::AllocateHistos()) return false;
    return true;
}

bool AgataSauronFwAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    AgataSauronHistograms& h = agataSauronHistograms;
    h.dir
        = input.outFile.mkdir("AgataSauronFw", "Agata-SauronFw coincidences");
    h.dir->cd();
    if(AgataSauronAnalysis::AllocateHistos()) return false;
    return true;
}

bool AgataSauronBwAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    AgataSauronHistograms& h = agataSauronHistograms;
    h.dir
        = input.outFile.mkdir("AgataSauronBw", "Agata-SauronBw coincidences");
    h.dir->cd();
    if(!AgataSauronAnalysis::AllocateHistos()) return false;
    return true;
}

void AgataSauronAnalysis::Analyze() { GammadetectorSauronAnalysis::Analyze(); }

void AgataSauronAnalysis::Finalize() {
    GammadetectorSauronAnalysis::Finalize();
}
