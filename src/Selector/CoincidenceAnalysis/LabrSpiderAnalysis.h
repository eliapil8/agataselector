#pragma once

#include "DetectorAnalysis/LabrAnalysis.h"
#include "DetectorAnalysis/SpiderAnalysis.h"
#include "GammadetectorSpiderAnalysis.h"
#include "Labr.h"
#include "Spider.h"

class LabrSpiderAnalysis : public GammadetectorSpiderAnalysis {
  public:
    LabrSpiderAnalysis(Inputs&, Labr&, Spider&, LabrAnalysis::LabrReader&,
                       SpiderAnalysis::SpiderReader&, std::vector<GammaDC>&);
    bool AllocateHistos() override;
    void Analyze() override;
    void Finalize() override;

  private:
    Labr&                     labr;
    Spider&                   spider;
    LabrAnalysis::LabrReader& labrReader;

  private:
    struct LabrSpiderHistograms : public GammadetectorSpiderHistograms {
        TH2D* m_TSspidermTSgamma_labrID{nullptr};
    } labrSpiderHistograms;
};
