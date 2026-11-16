#include "AgataPrismaDanteAnalysis.h"

AgataPrismaDanteAnalysis::AgataPrismaDanteAnalysis(
    DetectorAnalysis::Inputs& in, Agata& a, Prisma& p, Dante& d,
    AgataAnalysis::AgataReader& ra, PrismaAnalysis::PrismaReader& rp,
    DanteAnalysis::DanteReader& rd, GammaDC& gdc)
    : GammadetectorPrismaDanteAnalysis(
        in, ConfManager::getInstance().agataPrisma,
        ConfManager::getInstance().agataDante, a,
        ConfManager::getInstance().agata, p, d, ra, rp, rd, gdc,
        agataPrismaDanteHistograms),
      agata(a), prisma(p), dante(d), agataReader(ra) {
    if(coincidenceConf.enable_treeFill && input.type == Inputs::Type::SIGNAL && input.tree)
        input.tree->Branch("AgataPrismaDanteDC", &gdc);

    (void)agata;
    (void)prisma;
    (void)dante;
    (void)agataReader;
}

bool AgataPrismaDanteAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    AgataPrismaDanteHistograms& h = agataPrismaDanteHistograms;
    h.dir                         = input.outFile.mkdir("AgataPrismaDante",
                                                        "Agata-Prisma-Dante coincidences");
    h.dir->cd();
    GammadetectorPrismaDanteAnalysis::AllocateHistos();
    return true;
}

void AgataPrismaDanteAnalysis::Analyze() {
    GammadetectorPrismaDanteAnalysis::Analyze();
}

void AgataPrismaDanteAnalysis::Finalize() {
    GammadetectorPrismaDanteAnalysis::Finalize();
}
