#pragma once

#include "Agata.h"
#include "DetectorAnalysis/AgataAnalysis.h"
#include "DetectorAnalysis/SauronAnalysis.h"
#include "GammaDC.h"
#include "GammadetectorMcpSauronAnalysis.h"
#include "Sauron.h"
#include "Mcp.h"
#include "SauronMcp.h"

class AgataMcpSauronAnalysis : public GammadetectorMcpSauronAnalysis {
  protected:
    AgataMcpSauronAnalysis(Inputs&, Agata&, SauronMcp&, SauronConf&,
                        AgataSauronConf&, AgataAnalysis::AgataReader&,
                        SauronAnalysis::SauronReader&, std::vector<GammaDC>&, Sauron&, Mcp&);
    bool AllocateHistos() override;
    void Analyze() override;
    void Finalize() override;

  private:
    Agata&                      agata;
    Sauron&                     sauron;
    Mcp&                        mcp;
    SauronMcp&                  sauronMcp;
    const AgataSauronConf&      agataSauronConf;
    AgataAnalysis::AgataReader& agataReader;

  protected:
    struct AgataMcpSauronHistograms : public GammadetectorMcpSauronHistograms {
    } agataMcpSauronHistograms;
};

class AgataMcpSauronFwAnalysis : public AgataMcpSauronAnalysis {
  public:
    AgataMcpSauronFwAnalysis(Inputs&, Agata&, SauronMcp&,
                          AgataAnalysis::AgataReader&,
                          SauronAnalysis::SauronReader&,
                          std::vector<GammaDC>&, Sauron&, Mcp&);
    bool AllocateHistos() override;
};

class AgataMcpSauronBwAnalysis : public AgataMcpSauronAnalysis {
  public:
    AgataMcpSauronBwAnalysis(Inputs&, Agata&, SauronMcp&,
                          AgataAnalysis::AgataReader&,
                          SauronAnalysis::SauronReader&,
                          std::vector<GammaDC>&, Sauron&, Mcp&);
    bool AllocateHistos() override;
};
