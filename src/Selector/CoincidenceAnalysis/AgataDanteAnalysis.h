#pragma once

#include "Agata.h"
#include "Dante.h"
#include "DetectorAnalysis/AgataAnalysis.h"
#include "DetectorAnalysis/DanteAnalysis.h"
#include "GammaDC.h"
#include "GammadetectorDanteAnalysis.h"

class AgataDanteAnalysis : public GammadetectorDanteAnalysis {
  public:
    AgataDanteAnalysis(Inputs&, Agata&, Dante&, AgataAnalysis::AgataReader&,
                       DanteAnalysis::DanteReader&, std::vector<GammaDC>&);
    bool AllocateHistos() override;
    void Analyze() override;
    void Finalize() override;

  private:
    Agata&                      agata;
    Dante&                      dante;
    AgataAnalysis::AgataReader& agataReader;

  private:
    struct AgataDanteHistograms : public GammadetectorDanteHistograms {
    } agataDanteHistograms;
};
