#pragma once

#include "Agata.h"
#include "DetectorAnalysis/AgataAnalysis.h"
#include "DetectorAnalysis/SpiderAnalysis.h"
#include "GammaDC.h"
#include "GammadetectorSpiderAnalysis.h"
#include "Spider.h"

class AgataSpiderAnalysis : public GammadetectorSpiderAnalysis {
  public:
    AgataSpiderAnalysis(Inputs&, Agata&, Spider&, AgataAnalysis::AgataReader&,
                        SpiderAnalysis::SpiderReader&, std::vector<GammaDC>&);
    bool AllocateHistos() override;
    void Analyze() override;
    void Finalize() override;

  private:
    Agata&                      agata;
    Spider&                     spider;
    AgataAnalysis::AgataReader& agataReader;

  private:
    struct AgataSpiderHistograms : public GammadetectorSpiderHistograms {
    } agataSpiderHistograms;
};
