#pragma once

#include "CoincidenceAnalysis.h"
#include "DetectorAnalysis/EuclidesAnalysis.h"
#include "Euclides.h"
#include "GammaDC.h"
#include "Gammadetector.h"
#include "GammadetectorConf.h"
#include "GammadetectorEuclidesConf.h"

class GammadetectorEuclidesAnalysis : public CoincidenceAnalysis {
  protected:
    struct GammadetectorEuclidesHistograms : public CoincidenceHistograms {
        TH2D* m_TSdiff_TS{nullptr};
        // TH1D* h_TSeuclidesmTSgammadetector{nullptr};
        TH2D* m_TSeuclidesmTSgammadetector_euclidesID{nullptr};
        TH1D* h_E{nullptr};
        TH1D* h_EDC{nullptr};
        TH2D* m_E_E{nullptr};
        TH2D* m_EDC_EDC{nullptr};
        struct IonHistograms : public HistogramStructure {
            TH1D* h_E{nullptr};
            TH1D* h_EDCBP{nullptr};
            TH2D* m_E_E{nullptr};
            TH2D* m_EDCBP_EDCBP{nullptr};
        };
        struct ReactionHistograms : public HistogramStructure {
            TH1D* h_E{nullptr};
            TH1D* h_EDCBP{nullptr};
            TH2D* m_E_E{nullptr};
            TH2D* m_EDCBP_EDCBP{nullptr};
            TH2D* m_Ex_E{nullptr};
            TH2D* m_Ex_EDCBP{nullptr};
        };
        std::unordered_map<ReactionFragment::Ion, IonHistograms,
                           ReactionFragment::Ion::hash_fn>
            ionHistos;
        std::unordered_map<std::vector<ReactionFragment::Ion>,
                           ReactionHistograms,
                           ReactionFragment::Ion::vec_hash_fn>
            reactionHistos;
    };

  protected:
    GammadetectorEuclidesAnalysis(Inputs&, GammadetectorEuclidesConf&,
                                  Gammadetector&, GammadetectorConf&,
                                  Euclides&, Readers&,
                                  EuclidesAnalysis::EuclidesReader&, GammaDC&,
                                  GammadetectorEuclidesHistograms&);
    bool AllocateHistos() override;
    void Clear() override;
    void Analyze() override;
    bool FillHistos() override;
    void Finalize() override;

  private:
    Gammadetector&                    gammadetector;
    Euclides&                         euclides;
    EuclidesConf&                     euclidesConf;
    GammadetectorEuclidesConf&        gammadetectorEuclidesConf;
    GammadetectorConf&                gammadetectorConf;
    GammaDC&                          gammaDC;
    GammadetectorEuclidesHistograms&  gammadetectorEuclidesHistograms;
    EuclidesAnalysis::EuclidesReader& euclidesReader;
};
