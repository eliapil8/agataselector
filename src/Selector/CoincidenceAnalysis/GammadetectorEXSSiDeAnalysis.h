#pragma once

#include "CoincidenceAnalysis.h"
#include "DetectorAnalysis/EXSSiDeAnalysis.h"
#include "GammaDC.h"
#include "Gammadetector.h"
#include "GammadetectorConf.h"
#include "GammadetectorEXSSiDeConf.h"
#include "EXSSiDe.h"

class GammadetectorEXSSiDeAnalysis : public CoincidenceAnalysis {
  protected:
    struct GammadetectorEXSSiDeHistograms : public CoincidenceHistograms {
    };

  protected:
    GammadetectorEXSSiDeAnalysis(Inputs&, GammadetectorEXSSiDeConf&,
                                Gammadetector&, GammadetectorConf&, EXSSiDe&,
                                Readers&,
                                EXSSiDeAnalysis::EXSSiDeReader&,
                                std::vector<GammaDC>&,
                                GammadetectorEXSSiDeHistograms&);
    bool AllocateHistos() override;
    void Clear() override;
    void Analyze() override;
    bool FillHistos() override;
    void Finalize() override;

  private:
    Gammadetector&                 gammadetector;
    EXSSiDe&                        exsside;
    GammadetectorEXSSiDeConf&       gammadetectorEXSSiDeConf;
    GammadetectorConf&             gammadetectorConf;
    std::vector<GammaDC>&          gammaDC;
    GammadetectorEXSSiDeHistograms& gammadetectorEXSSiDeHistograms;
    EXSSiDeAnalysis::EXSSiDeReader&  exssideReader;
};
