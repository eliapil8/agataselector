#pragma once

#include "CoincidenceAnalysis.h"
#include "Dante.h"
#include "DetectorAnalysis/DanteAnalysis.h"
#include "GammaDC.h"
#include "Gammadetector.h"
#include "GammadetectorConf.h"
#include "GammadetectorDanteConf.h"

class GammadetectorDanteAnalysis : public CoincidenceAnalysis {
  protected:
    struct GammadetectorDanteHistograms : public CoincidenceHistograms {
        TH2D* m_TSdiff_TS{nullptr};
        TH1D* h_cTime_gated{nullptr};
        // TH1D* h_TSdantemTSgamma{nullptr};
        TH2D* m_TSdantemTSgamma_danteID{nullptr};
        TH1D* h_E{nullptr};
        TH1D* h_EDCBP{nullptr};
        TH1D* h_beta_BP{nullptr};
        TH2D* h_p4_theta_phi{nullptr};
    };

  protected:
    GammadetectorDanteAnalysis(Inputs&, GammadetectorDanteConf&,
                               Gammadetector&, GammadetectorConf&, Dante&,
                               Readers&, DanteAnalysis::DanteReader&,
                               std::vector<GammaDC>&,
                               GammadetectorDanteHistograms&);
    bool AllocateHistos() override;
    void Clear() override;
    void Analyze() override;
    bool FillHistos() override;
    void Finalize() override;

  private:
    Gammadetector&                gammadetector;
    Dante&                        dante;
    GammadetectorDanteConf&       gammadetectorDanteConf;
    GammadetectorConf&            gammadetectorConf;
    std::vector<GammaDC>&         gammaDC;
    GammadetectorDanteHistograms& gammadetectorDanteHistograms;
    DanteAnalysis::DanteReader&   danteReader;
};
