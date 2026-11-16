#pragma once

#include "DetectorAnalysis/LabrAnalysis.h"
#include "DetectorAnalysis/PrismaAnalysis.h"
#include "GammadetectorPrismaAnalysis.h"
#include "Labr.h"
#include "Prisma.h"

class LabrPrismaAnalysis : public GammadetectorPrismaAnalysis {
  public:
    LabrPrismaAnalysis(Inputs&, Labr&, Prisma&, LabrAnalysis::LabrReader&,
                       PrismaAnalysis::PrismaReader&, GammaDC&);
    bool AllocateHistos() override;
    void Analyze() override;
    void Finalize() override;

  private:
    Labr&                     labr;
    Prisma&                   prisma;
    LabrAnalysis::LabrReader& labrReader;

  private:
    struct LabrPrismaHistograms : public GammadetectorPrismaHistograms {
        TH2D* m_TSprismamTSgamma_labrID{nullptr};
    } labrPrismaHistograms;
};
