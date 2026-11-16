#pragma once

#include "Agata.h"
#include "DetectorAnalysis/AgataAnalysis.h"
#include "DetectorAnalysis/EXSSiDeAnalysis.h"
#include "GammaDC.h"
#include "GammadetectorEXSSiDeAnalysis.h"
#include "EXSSiDe.h"

class AgataEXSSiDeAnalysis : public GammadetectorEXSSiDeAnalysis {
  public:
    AgataEXSSiDeAnalysis(Inputs&, Agata&, EXSSiDe&, AgataAnalysis::AgataReader&,
                        EXSSiDeAnalysis::EXSSiDeReader&, std::vector<GammaDC>&);
    bool AllocateHistos() override;
    void Analyze() override;
    void Finalize() override;

  private:
    Agata&                      agata;
    EXSSiDe&                     exsside;
    AgataAnalysis::AgataReader& agataReader;

  private:
    struct AgataEXSSiDeHistograms : public GammadetectorEXSSiDeHistograms {
    } agataEXSSiDeHistograms;
};