#pragma once

#include "Agata.h"
#include "Dante.h"
#include "DetectorAnalysis/AgataAnalysis.h"
#include "DetectorAnalysis/DanteAnalysis.h"
#include "DetectorAnalysis/PrismaAnalysis.h"
#include "GammadetectorPrismaDanteAnalysis.h"
#include "Prisma.h"

class AgataPrismaDanteAnalysis : public GammadetectorPrismaDanteAnalysis {
  public:
    AgataPrismaDanteAnalysis(Inputs&, Agata&, Prisma&, Dante&,
                             AgataAnalysis::AgataReader&,
                             PrismaAnalysis::PrismaReader&,
                             DanteAnalysis::DanteReader&, GammaDC&);
    bool AllocateHistos() override;
    void Analyze() override;
    void Finalize() override;

  private:
    Agata&                      agata;
    Prisma&                     prisma;
    Dante&                      dante;
    AgataAnalysis::AgataReader& agataReader;

  private:
    struct AgataPrismaDanteHistograms
        : public GammadetectorPrismaDanteHistograms {
    } agataPrismaDanteHistograms;
};
