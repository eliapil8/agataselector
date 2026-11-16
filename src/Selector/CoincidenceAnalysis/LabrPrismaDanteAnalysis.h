#pragma once

#include "Dante.h"
#include "DetectorAnalysis/DanteAnalysis.h"
#include "DetectorAnalysis/LabrAnalysis.h"
#include "DetectorAnalysis/PrismaAnalysis.h"
#include "GammadetectorPrismaDanteAnalysis.h"
#include "Labr.h"
#include "Prisma.h"

class LabrPrismaDanteAnalysis : public GammadetectorPrismaDanteAnalysis {
  public:
    LabrPrismaDanteAnalysis(Inputs&, Labr&, Prisma&, Dante&,
                            LabrAnalysis::LabrReader&,
                            PrismaAnalysis::PrismaReader&,
                            DanteAnalysis::DanteReader&, GammaDC&);
    bool AllocateHistos() override;
    void Analyze() override;
    void Finalize() override;

  private:
    Labr&                     labr;
    Prisma&                   prisma;
    Dante&                    dante;
    LabrAnalysis::LabrReader& labrReader;

  private:
    struct LabrPrismaDanteHistograms
        : public GammadetectorPrismaDanteHistograms {
    } labrPrismaDanteHistograms;
};
