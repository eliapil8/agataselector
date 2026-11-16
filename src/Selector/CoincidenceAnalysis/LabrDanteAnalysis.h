#pragma once

#include "Dante.h"
#include "DetectorAnalysis/DanteAnalysis.h"
#include "DetectorAnalysis/LabrAnalysis.h"
#include "GammadetectorDanteAnalysis.h"
#include "Labr.h"

class LabrDanteAnalysis : public GammadetectorDanteAnalysis {
  public:
    LabrDanteAnalysis(Inputs&, Labr&, Dante&, LabrAnalysis::LabrReader&,
                      DanteAnalysis::DanteReader&, std::vector<GammaDC>&);
    bool AllocateHistos() override;
    void Analyze() override;
    void Finalize() override;

  private:
    Labr&                     labr;
    Dante&                    dante;
    LabrAnalysis::LabrReader& labrReader;

  private:
    struct LabrDanteHistograms : public GammadetectorDanteHistograms {
        TH2D* m_TSdantemTSgamma_labrID{nullptr};
    } labrDanteHistograms;
};
