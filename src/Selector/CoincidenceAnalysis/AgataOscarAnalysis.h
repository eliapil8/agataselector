#pragma once

#include "Agata.h"
#include "DetectorAnalysis/AgataAnalysis.h"
#include "DetectorAnalysis/OscarAnalysis.h"
#include "GammaDC.h"
#include "GammadetectorOscarAnalysis.h"
#include "Oscar.h"

class AgataOscarAnalysis : public GammadetectorOscarAnalysis {
  protected:
    AgataOscarAnalysis(Inputs&, Agata&, Oscar&, OscarConf&, AgataOscarConf&,
                       AgataAnalysis::AgataReader&,
                       OscarAnalysis::OscarReader&, std::vector<GammaDC>&);
    bool AllocateHistos() override;
    void Analyze() override;
    void Finalize() override;

  private:
    Agata&                      agata;
    Oscar&                      oscar;
    const AgataOscarConf&       agataOscarConf;
    AgataAnalysis::AgataReader& agataReader;

  protected:
    struct AgataOscarHistograms : public GammadetectorOscarHistograms {
    } agataOscarHistograms;
};

class AgataOscarUpAnalysis : public AgataOscarAnalysis {
  public:
    AgataOscarUpAnalysis(Inputs&, Agata&, Oscar&, AgataAnalysis::AgataReader&,
                         OscarAnalysis::OscarReader&, std::vector<GammaDC>&);
    bool AllocateHistos() override;
};

class AgataOscarDownAnalysis : public AgataOscarAnalysis {
  public:
    AgataOscarDownAnalysis(Inputs&, Agata&, Oscar&,
                           AgataAnalysis::AgataReader&,
                           OscarAnalysis::OscarReader&, std::vector<GammaDC>&);
    bool AllocateHistos() override;
};
