#include "AgataPrismaAnalysis.h"

AgataPrismaAnalysis::AgataPrismaAnalysis(DetectorAnalysis::Inputs& in,
                                         Agata& a, Prisma& p,
                                         AgataAnalysis::AgataReader&   ra,
                                         PrismaAnalysis::PrismaReader& rp,
                                         GammaDC&                      gdc)
    : GammadetectorPrismaAnalysis(in, ConfManager::getInstance().agataPrisma,
                                  a, ConfManager::getInstance().agata, p, ra,
                                  rp, gdc, agataPrismaHistograms),
      agata(a), prisma(p), agataReader(ra) {
    if(coincidenceConf.enable_treeFill && input.type == Inputs::Type::SIGNAL && input.tree && coincidenceConf.enable_redBranch)
        input.tree->Branch("AgataPrismaDC", &gdc.redgammaDC);
    else if (coincidenceConf.enable_treeFill && input.type == Inputs::Type::SIGNAL && input.tree)
        input.tree->Branch("AgataPrismaDC", &gdc);

    (void)agata;
    (void)prisma;
    (void)agataReader;
}

bool AgataPrismaAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    AgataPrismaHistograms& h = agataPrismaHistograms;
    h.dir = input.outFile.mkdir("AgataPrisma", "Agata-Prisma coincidences");
    h.dir->cd();
    GammadetectorPrismaAnalysis::AllocateHistos();

    return true;
}

void AgataPrismaAnalysis::Analyze() { GammadetectorPrismaAnalysis::Analyze(); }

void AgataPrismaAnalysis::Finalize() {
    GammadetectorPrismaAnalysis::Finalize();
}
