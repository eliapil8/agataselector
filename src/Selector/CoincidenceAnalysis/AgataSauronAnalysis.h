#pragma once

#include "Agata.h"
#include "DetectorAnalysis/AgataAnalysis.h"
#include "DetectorAnalysis/SauronAnalysis.h"
#include "GammaDC.h"
#include "GammadetectorSauronAnalysis.h"
#include "Sauron.h"

class AgataSauronAnalysis : public GammadetectorSauronAnalysis {
  protected:
    AgataSauronAnalysis(Inputs&, Agata&, Sauron&, SauronConf&,
                        AgataSauronConf&, AgataAnalysis::AgataReader&,
                        SauronAnalysis::SauronReader&, std::vector<GammaDC>&);
    bool AllocateHistos() override;
    void Analyze() override;
    void Finalize() override;

  private:
    Agata&                      agata;
    Sauron&                     sauron;
    const AgataSauronConf&      agataSauronConf;
    AgataAnalysis::AgataReader& agataReader;

  protected:
    struct AgataSauronHistograms : public GammadetectorSauronHistograms {
    } agataSauronHistograms;
};

class AgataSauronFwAnalysis : public AgataSauronAnalysis {
  public:
    AgataSauronFwAnalysis(Inputs&, Agata&, Sauron&,
                          AgataAnalysis::AgataReader&,
                          SauronAnalysis::SauronReader&,
                          std::vector<GammaDC>&);
    bool AllocateHistos() override;
};

class AgataSauronBwAnalysis : public AgataSauronAnalysis {
  public:
    AgataSauronBwAnalysis(Inputs&, Agata&, Sauron&,
                          AgataAnalysis::AgataReader&,
                          SauronAnalysis::SauronReader&,
                          std::vector<GammaDC>&);
    bool AllocateHistos() override;
};
