#include "LabrPrismaDanteAnalysis.h"

LabrPrismaDanteAnalysis::LabrPrismaDanteAnalysis(
    DetectorAnalysis::Inputs& in, Labr& l, Prisma& p, Dante& d,
    LabrAnalysis::LabrReader& rl, PrismaAnalysis::PrismaReader& rp,
    DanteAnalysis::DanteReader& rd, GammaDC& gdc)
    : GammadetectorPrismaDanteAnalysis(
        in, ConfManager::getInstance().labrPrisma,
        ConfManager::getInstance().labrDante, l,
        ConfManager::getInstance().labr, p, d, rl, rp, rd, gdc,
        labrPrismaDanteHistograms),
      labr(l), prisma(p), dante(d), labrReader(rl) {
    if(coincidenceConf.enable_treeFill && input.type == Inputs::Type::SIGNAL && input.tree)
        input.tree->Branch("LabrPrismaDanteDC", &gdc);

    (void)labr;
    (void)prisma;
    (void)dante;
    (void)labrReader;
}

bool LabrPrismaDanteAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    LabrPrismaDanteHistograms& h = labrPrismaDanteHistograms;
    h.dir                        = input.outFile.mkdir("LabrPrismaDante",
                                                       "Labr-Prisma-Dante coincidences");
    h.dir->cd();
    GammadetectorPrismaDanteAnalysis::AllocateHistos();
    return true;
}

void LabrPrismaDanteAnalysis::Analyze() {
    GammadetectorPrismaDanteAnalysis::Analyze();
}

void LabrPrismaDanteAnalysis::Finalize() {
    GammadetectorPrismaDanteAnalysis::Finalize();
}
