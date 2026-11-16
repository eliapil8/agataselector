#pragma once

#include "DetectorAnalysis/LabrAnalysis.h"
#include "DetectorAnalysis/SauronAnalysis.h"
#include "GammaDC.h"
#include "GammadetectorSauronAnalysis.h"
#include "Labr.h"
#include "Sauron.h"

class LabrSauronAnalysis : public GammadetectorSauronAnalysis {
  protected:
    LabrSauronAnalysis(Inputs&, Labr&, Sauron&, SauronConf&, LabrSauronConf&,
                       LabrAnalysis::LabrReader&,
                       SauronAnalysis::SauronReader&, std::vector<GammaDC>&);
    bool AllocateHistos() override;
    void Analyze() override;
    void Finalize() override;

  private:
    Labr&                     labr;
    Sauron&                   sauron;
    const LabrSauronConf&     labrSauronConf;
    LabrAnalysis::LabrReader& labrReader;

  protected:
    struct LabrSauronHistograms : public GammadetectorSauronHistograms {
    } labrSauronHistograms;
};

class LabrSauronFwAnalysis : public LabrSauronAnalysis {
  public:
    LabrSauronFwAnalysis(Inputs&, Labr&, Sauron&, LabrAnalysis::LabrReader&,
                         SauronAnalysis::SauronReader&, std::vector<GammaDC>&);
    bool AllocateHistos() override;
};

class LabrSauronBwAnalysis : public LabrSauronAnalysis {
  public:
    LabrSauronBwAnalysis(Inputs&, Labr&, Sauron&, LabrAnalysis::LabrReader&,
                         SauronAnalysis::SauronReader&, std::vector<GammaDC>&);
    bool AllocateHistos() override;
};
