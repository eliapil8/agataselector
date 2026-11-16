#pragma once

#include "DetectorAnalysis/LabrAnalysis.h"
#include "DetectorAnalysis/EXSSiDeAnalysis.h"
#include "GammaDC.h"
#include "GammadetectorEXSSiDeAnalysis.h"
#include "Labr.h"
#include "EXSSiDe.h"

class LabrEXSSiDeAnalysis : public GammadetectorEXSSiDeAnalysis {
  public:
    LabrEXSSiDeAnalysis(Inputs&, Labr&, EXSSiDe&,
                       LabrAnalysis::LabrReader&,
                       EXSSiDeAnalysis::EXSSiDeReader&, std::vector<GammaDC>&);
    bool AllocateHistos() override;
    void Analyze() override;
    void Finalize() override;

  private:
    Labr&                     labr;
    EXSSiDe&                   exsside;
    LabrAnalysis::LabrReader& labrReader;

  private:
    struct LabrEXSSiDeHistograms : public GammadetectorEXSSiDeHistograms {
    } labrEXSSiDeHistograms;
};
