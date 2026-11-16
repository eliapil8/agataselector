#pragma once

#include "DetectorAnalysis/LabrAnalysis.h"
#include "DetectorAnalysis/OscarAnalysis.h"
#include "GammaDC.h"
#include "GammadetectorOscarAnalysis.h"
#include "Labr.h"
#include "Oscar.h"

class LabrOscarAnalysis : public GammadetectorOscarAnalysis {
  protected:
    LabrOscarAnalysis(Inputs&, Labr&, Oscar&, OscarConf&, LabrOscarConf&,
                      LabrAnalysis::LabrReader&, OscarAnalysis::OscarReader&,
                      std::vector<GammaDC>&);
    bool AllocateHistos() override;
    void Analyze() override;
    void Finalize() override;

  private:
    Labr&                     labr;
    Oscar&                    oscar;
    const LabrOscarConf&      labrOscarConf;
    LabrAnalysis::LabrReader& labrReader;

  protected:
    struct LabrOscarHistograms : public GammadetectorOscarHistograms {
    } labrOscarHistograms;
};

class LabrOscarUpAnalysis : public LabrOscarAnalysis {
  public:
    LabrOscarUpAnalysis(Inputs&, Labr&, Oscar&, LabrAnalysis::LabrReader&,
                        OscarAnalysis::OscarReader&, std::vector<GammaDC>&);
    bool AllocateHistos() override;
};

class LabrOscarDownAnalysis : public LabrOscarAnalysis {
  public:
    LabrOscarDownAnalysis(Inputs&, Labr&, Oscar&, LabrAnalysis::LabrReader&,
                          OscarAnalysis::OscarReader&, std::vector<GammaDC>&);
    bool AllocateHistos() override;
};
