#include "AgataMcpSauronAnalysis.h"

AgataMcpSauronAnalysis::AgataMcpSauronAnalysis(DetectorAnalysis::Inputs& in,
                                         Agata& a, SauronMcp& sm, SauronConf& sc,
                                         AgataSauronConf&              c,
                                         AgataAnalysis::AgataReader&   ra,
                                         SauronAnalysis::SauronReader& rs,
                                         std::vector<GammaDC>&         gdc, Sauron& s, Mcp& mm)
    : GammadetectorMcpSauronAnalysis(in, c, a, ConfManager::getInstance().agata,
                                  sm, sc, ra, rs, gdc, agataMcpSauronHistograms, s , mm),
      agata(a), sauron(s), mcp(mm), sauronMcp(sm) , agataSauronConf(c), agataReader(ra) {
        (void)agata;
        (void)sauron;
        (void)agataSauronConf;
        (void)agataReader;
    }

AgataMcpSauronFwAnalysis::AgataMcpSauronFwAnalysis(DetectorAnalysis::Inputs& in,
                                             Agata& a, SauronMcp& sm,
                                             AgataAnalysis::AgataReader&   ra,
                                             SauronAnalysis::SauronReader& rs,
                                             std::vector<GammaDC>&         gdc, Sauron& s, Mcp& mm)
    : AgataMcpSauronAnalysis(in, a, sm, ConfManager::getInstance().sauronFw,
                          ConfManager::getInstance().agataSauronFw, ra, rs,
                          gdc, s, mm) {
    if(coincidenceConf.enable_treeFill && input.type == Inputs::Type::SIGNAL && input.tree)
        input.tree->Branch("AgataMcpSauronFwDC", &gdc);
}

AgataMcpSauronBwAnalysis::AgataMcpSauronBwAnalysis(DetectorAnalysis::Inputs& in,
                                             Agata& a, SauronMcp& sm,
                                             AgataAnalysis::AgataReader&   ra,
                                             SauronAnalysis::SauronReader& rs,
                                             std::vector<GammaDC>&         gdc, Sauron& s, Mcp& mm)
    : AgataMcpSauronAnalysis(in, a, sm, ConfManager::getInstance().sauronBw,
                          ConfManager::getInstance().agataSauronBw, ra, rs,
                          gdc, s, mm) {
    if(coincidenceConf.enable_treeFill)
        input.tree->Branch("AgataMcpSauronBwDC", &gdc);
}

bool AgataMcpSauronAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    if(!GammadetectorMcpSauronAnalysis::AllocateHistos()) return false;
    return true;
}

bool AgataMcpSauronFwAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    AgataMcpSauronHistograms& h = agataMcpSauronHistograms;
    h.dir
        = input.outFile.mkdir("AgataMcpSauronFw", "Agata-SauronFw-Mcp coincidences");
    h.dir->cd();
    if(AgataMcpSauronAnalysis::AllocateHistos()) return false;
    return true;
}

bool AgataMcpSauronBwAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    AgataMcpSauronHistograms& h = agataMcpSauronHistograms;
    h.dir
        = input.outFile.mkdir("AgataMcpSauronBw", "Agata-SauronBw-Mcp coincidences");
    h.dir->cd();
    if(!AgataMcpSauronAnalysis::AllocateHistos()) return false;
    return true;
}

void AgataMcpSauronAnalysis::Analyze() { GammadetectorMcpSauronAnalysis::Analyze(); }

void AgataMcpSauronAnalysis::Finalize() {
    GammadetectorMcpSauronAnalysis::Finalize();
}
