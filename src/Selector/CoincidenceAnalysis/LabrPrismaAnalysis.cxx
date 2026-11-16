#include "LabrPrismaAnalysis.h"

LabrPrismaAnalysis::LabrPrismaAnalysis(DetectorAnalysis::Inputs& in, Labr& l,
                                       Prisma& p, LabrAnalysis::LabrReader& rl,
                                       PrismaAnalysis::PrismaReader& rp,
                                       GammaDC&                      gdc)
    : GammadetectorPrismaAnalysis(in, ConfManager::getInstance().labrPrisma, l,
                                  ConfManager::getInstance().labr, p, rl, rp,
                                  gdc, labrPrismaHistograms),
      labr(l), prisma(p), labrReader(rl) {
    if(coincidenceConf.enable_treeFill && input.type == Inputs::Type::SIGNAL && input.tree)
        input.tree->Branch("LabrPrismaDC", &gdc);

    (void)labr;
    (void)prisma;
    (void)labrReader;
}

bool LabrPrismaAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    LabrPrismaHistograms& h = labrPrismaHistograms;
    h.dir = input.outFile.mkdir("LabrPrisma", "Labr-Prisma coincidences");
    h.dir->cd();
    GammadetectorPrismaAnalysis::AllocateHistos();

    Initialize<TH2D>(
        h.m_TSprismamTSgamma_labrID,
        new TH2D("m_TSprismamTSgamma_labrID",
                 "PrismaTS - GammaTS vs LaBr ID; LaBr ID; TS "
                 "difference [10 ns]; Counts",
                 64, 0, 64,
                 (ConfManager::getInstance().labrPrisma.windowRight
                  - ConfManager::getInstance().labrPrisma.windowLeft + 200)
                     * 10,
                 ConfManager::getInstance().labrPrisma.windowLeft - 100,
                 ConfManager::getInstance().labrPrisma.windowRight + 100),
        h.ptrs);
    return true;
}

void LabrPrismaAnalysis::Analyze() { GammadetectorPrismaAnalysis::Analyze(); }

void LabrPrismaAnalysis::Finalize() {
    GammadetectorPrismaAnalysis::Finalize();
}
