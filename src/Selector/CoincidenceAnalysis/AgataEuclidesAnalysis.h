#pragma once

#include "Agata.h"
#include "DetectorAnalysis/AgataAnalysis.h"
#include "DetectorAnalysis/EuclidesAnalysis.h"
#include "Euclides.h"
#include "GammaDC.h"
#include "GammadetectorEuclidesAnalysis.h"

class AgataEuclidesAnalysis : public GammadetectorEuclidesAnalysis {
  public:
    AgataEuclidesAnalysis(Inputs&, Agata&, Euclides&,
                          AgataAnalysis::AgataReader&,
                          EuclidesAnalysis::EuclidesReader&, GammaDC&);
    bool AllocateHistos() override;
    void Analyze() override;
    bool FillHistos() override;
    void Finalize() override;

  private:
    Agata&                      agata;
    Euclides&                   euclides;
    AgataAnalysis::AgataReader& agataReader;

  private:
    struct AgataEuclidesHistograms : public GammadetectorEuclidesHistograms {
    } agataEuclidesHistograms;
};
