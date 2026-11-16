#pragma once

#include "DetectorAnalysis/EuclidesAnalysis.h"
#include "DetectorAnalysis/LabrAnalysis.h"
#include "Euclides.h"
#include "GammadetectorEuclidesAnalysis.h"
#include "Labr.h"

class LabrEuclidesAnalysis : public GammadetectorEuclidesAnalysis {
  public:
    LabrEuclidesAnalysis(Inputs&, Labr&, Euclides&, LabrAnalysis::LabrReader&,
                         EuclidesAnalysis::EuclidesReader&, GammaDC&);
    bool AllocateHistos() override;
    void Analyze() override;
    void Finalize() override;

  private:
    Labr&                     labr;
    Euclides&                 euclides;
    LabrAnalysis::LabrReader& labrReader;

  private:
    struct LabrEuclidesHistograms : public GammadetectorEuclidesHistograms {
        TH2D* m_TSeuclidesmTSgamma_labrID{nullptr};
    } labrEuclidesHistograms;
};
