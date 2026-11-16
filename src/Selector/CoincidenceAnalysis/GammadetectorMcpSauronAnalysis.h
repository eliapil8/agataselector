#pragma once

#include "CoincidenceAnalysis.h"
#include "DetectorAnalysis/SauronAnalysis.h"
#include "DetectorAnalysis/McpAnalysis.h"
#include "McpSauronAnalysis.h"
#include "GammaDC.h"
#include "Gammadetector.h"
#include "GammadetectorConf.h"
#include "GammadetectorSauronConf.h"
#include "McpSauronConf.h"
#include "Sauron.h"
#include "SauronMcp.h"
#include "Mcp.h"

class GammadetectorMcpSauronAnalysis : public CoincidenceAnalysis {
  protected:
  struct GammadetectorMcpSauronHistograms : public CoincidenceHistograms {
    // TH2D* m_TSdiff_TS{nullptr};
    TH1D* h_cTime_gated{nullptr};
    // TH1D* h_TSsauronmTSgammadetector{nullptr};
    TH2D* m_TSsauronmTSgammadetector_sauronID{nullptr};
    TH1D* h_E{nullptr};
    TH1D* h_EDC{nullptr};
    TH1D* h_EDCBP{nullptr};
    TH2D* m_E_Beta{nullptr};
    TH2D* m_E_BetaBP{nullptr};
    TH2D* m_EDC_Beta{nullptr};
    TH2D* m_EDCBP_BetaBP{nullptr};
    TH2D* m_E_Theta{nullptr};
    TH2D* m_E_ThetaBP{nullptr};
    TH2D* m_ESauron_Theta_pg{nullptr};
    TH2D* m_EDCBP_Theta{nullptr};
    TH2D* m_EDC_Theta{nullptr};
    TH2D* m_EDCBP_ThetaBP{nullptr};
    TH2D* m_EDCBP_TS{nullptr}; // Added by MR (be careful!)
    TH2D* m_Ex_E{nullptr};
    TH2D* m_Ex_EDC{nullptr};
    TH2D* m_Ex_EDCBP{nullptr};
    TH2D* m_SauronID_EDC{nullptr};
    TH2D* m_SauronID_EDCBP{nullptr};
    TH2D* m_E_TSmTS_cfd{nullptr};
    TH2D* m_EDC_TSmTS_cfd{nullptr};
    TH2D* m_Ex_TSmTS{nullptr};
    TH2D* m_E_posX{nullptr};
    TH2D* m_E_posY{nullptr};

    struct ExSelection : public HistogramStructure {
            TH1D*                     h_E{nullptr};
            TH1D*                     h_EDC{nullptr};
            TH1D*                     h_EDCBP{nullptr};
            TH2D*                     m_E_Theta{nullptr};
            TH2D*                     m_E_ThetaBP{nullptr};
            TH2D*                     m_EDCBP_Theta{nullptr};
            TH2D*                     m_EDCBP_ThetaBP{nullptr};
            TH2D*                     m_E_Beta{nullptr};
            TH2D*                     m_E_BetaBP{nullptr};
            TH2D* m_EDC_TSmTS_cfd{nullptr};
            std::pair<double, double> vals{0, 0};
            TGraph*                   g_Energy_Theta{nullptr};
            TGraph*                   g_Energy_Theta2{nullptr};
            // std::vector<GammaGatedHistograms> gammagatedHistograms;
        };

        std::vector<ExSelection> exSelection;
        std::vector<ExSelection> exSelectionCbC;
        struct ReactionHistograms : public HistogramStructure {
	  TH1D* h_E{nullptr};
	  TH1D* h_EDCBP{nullptr};
	  TH2D* m_Ex_E{nullptr};
	  TH2D* m_Ex_EDCBP{nullptr};
	  TH2D* m_SauronID_Ex{nullptr};

        };
    std::unordered_map<std::vector<ReactionFragment::Ion>,
                           ReactionHistograms,
                           ReactionFragment::Ion::vec_hash_fn>
            reactionHistos;
    };

  protected:
    GammadetectorMcpSauronAnalysis(Inputs&, GammadetectorSauronConf&,
                                Gammadetector&, GammadetectorConf&, SauronMcp&,
                                SauronConf&, Readers&,
                                SauronAnalysis::SauronReader&,
                                std::vector<GammaDC>&,
                                GammadetectorMcpSauronHistograms&, Sauron&, Mcp&);
    bool AllocateHistos() override;
    void Clear() override;
    void Analyze() override;
    bool FillHistos() override;
    void Finalize() override;

  private:
    Gammadetector&                 gammadetector;
    Sauron&                        sauron;
    SauronMcp&                     sauronMcp;
    Mcp&                           mcp;
    SauronConf&                    sauronConf;
    GammadetectorSauronConf&       gammadetectorSauronConf;
    GammadetectorConf&             gammadetectorConf;
    std::vector<GammaDC>&          gammaDC;
    GammadetectorMcpSauronHistograms& gammadetectorMcpSauronHistograms;
    SauronAnalysis::SauronReader&  sauronReader;

  protected:
  private:
    TGraph*      GenerateKinematicLine(const double&);
    unsigned int kineLineCounter{0};
};
