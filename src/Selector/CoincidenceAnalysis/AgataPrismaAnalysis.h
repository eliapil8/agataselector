#pragma once

#include "Agata.h"
#include "DetectorAnalysis/AgataAnalysis.h"
#include "DetectorAnalysis/PrismaAnalysis.h"
#include "GammadetectorPrismaAnalysis.h"
#include "Prisma.h"

class AgataPrismaAnalysis : public GammadetectorPrismaAnalysis {
  public:
    AgataPrismaAnalysis(Inputs&, Agata&, Prisma&, AgataAnalysis::AgataReader&,
                        PrismaAnalysis::PrismaReader&, GammaDC&);
    bool AllocateHistos() override;
    void Analyze() override;
    void Finalize() override;

  private:
    Agata&                      agata;
    Prisma&                     prisma;
    AgataAnalysis::AgataReader& agataReader;

  private:
    struct AgataPrismaHistograms : public GammadetectorPrismaHistograms {
    } agataPrismaHistograms;
};
