#pragma once

#include "CoincidenceAnalysis.h"
#include "DetectorAnalysis/PrismaAnalysis.h"
#include "GammaDC.h"
#include "Gammadetector.h"
#include "GammadetectorConf.h"
#include "GammadetectorPrismaConf.h"
#include "Prisma.h"

class GammadetectorPrismaAnalysis : public CoincidenceAnalysis {
  protected:
    struct GammadetectorPrismaHistograms : public CoincidenceHistograms {
        TH2D* m_TSdiff_TS{nullptr};
        TH1D* h_cTime{nullptr};
        TH1D* h_cTime_gated{nullptr};
        TH2D* m_cTime_coreId{nullptr};
        TH2D* m_cTime_E{nullptr};
        TH2D* m_cTime_EDC{nullptr};
        TH2D* m_cTime_EDCBP{nullptr};
        //TH1D* h_TSprismamTSgamma{nullptr};
        TH2D* m_ICDEAB_ICE{nullptr};
        TH1D* h_E{nullptr};
        TH1D* h_DC{nullptr};
        TH1D* h_DCBP{nullptr};
        TH2D* m_E_E{nullptr};
        TH2D* m_DC_DC{nullptr};
        TH2D* m_DCBP_DCBP{nullptr};
        TH2D* m_E_Theta{nullptr};
        TH2D* m_EBP_Theta{nullptr};
        TH2D* m_EDC_Theta{nullptr};
        TH2D* m_EDCBP_Theta{nullptr};

        struct IonHistograms : public HistogramStructure {
            TH1D* h_noDC_ion{nullptr};
            TH1D* h_DC_ion{nullptr};
            TH1D* h_DCBP_ion{nullptr};
            TH2D* m_Beta_BetaBP_ion{nullptr};
            TH2D* m_Theta_ThetaBP_ion{nullptr};
            TH2D* m_noDC_noDC_ion{nullptr};
            TH2D* m_DC_DC_ion{nullptr};
            TH2D* m_DCBP_DCBP_ion{nullptr};
            TH2D* m_noDC_theta_ion{nullptr};
            TH2D* m_noDC_thetaBP_ion{nullptr};
            TH2D* m_DC_theta_ion{nullptr};
            TH2D* m_DCBP_theta_ion{nullptr};
            TH2D* m_noDC_Qval_ion{nullptr};
            TH2D* m_DC_Qval_ion{nullptr};
            TH2D* m_DCBP_Qval_ion{nullptr};
            TH2D* m_dBeta_EDC_ion{nullptr};
            TH2D* m_dTOF_EDC_ion{nullptr};
            TH2D* m_colNr_EDC_ion{nullptr};
            TH2D* m_Xfp_EDC_ion{nullptr};
            TH2D* m_mcpX_EDC_ion{nullptr};
            TH2D* m_mcpY_EDC_ion{nullptr};
            struct ExSelection : public HistogramStructure {
                TH1D*                     h_E{nullptr};
                TH1D*                     h_EDC{nullptr};
                TH1D*                     h_EDCBP{nullptr};
                TH2D*                     m_E_Theta{nullptr};
                TH2D*                     m_E_ThetaBP{nullptr};
                TH2D*                     m_EDCBP_Theta{nullptr};
                TH2D*                     m_EDCBP_ThetaBP{nullptr};
                TH2D*                     m_E_E{nullptr};
                TH2D*                     m_EDC_EDC{nullptr};
                TH2D*                     m_EDCBP_EDCBP{nullptr};
                std::pair<double, double> vals{0, 0};
            };
            std::vector<ExSelection> exSelection;
        };
        std::unordered_map<ReactionFragment::Ion, IonHistograms,
                           ReactionFragment::Ion::hash_fn>
            ionHistos;

        struct DE_E : public HistogramStructure {
            TH1D*  h_noDC_cut{nullptr};
            TH1D*  h_DC_cut{nullptr};
            TH1D*  h_DCBP_cut{nullptr};
            TH2D*  m_Beta_BetaBP_cut{nullptr};
            TH2D*  m_Theta_ThetaBP_cut{nullptr};
            TH2D*  m_noDC_noDC_cut{nullptr};
            TH2D*  m_DC_DC_cut{nullptr};
            TH2D*  m_DCBP_DCBP_cut{nullptr};
            TH2D*  m_noDC_theta_cut{nullptr};
            TH2D*  m_noDC_thetaBP_cut{nullptr};
            TH2D*  m_DC_theta_cut{nullptr};
            TH2D*  m_DCBP_theta_cut{nullptr};
            TH2D*  m_noDC_Qval_cut{nullptr};
            TH2D*  m_DC_Qval_cut{nullptr};
            TH2D*  m_DCBP_Qval_cut{nullptr};
            TH2D*  m_dBeta_EDC_cut{nullptr};
            TH2D*  m_dTOF_EDC_cut{nullptr};
            TCutG* cut;
            explicit DE_E(TCutG* cut) : cut(cut){};
        };
        std::vector<DE_E> cutHistos;
    };

  protected:
    GammadetectorPrismaAnalysis(Inputs&, GammadetectorPrismaConf&,
                                Gammadetector&, GammadetectorConf&, Prisma&,
                                Readers&, PrismaAnalysis::PrismaReader&,
                                GammaDC&, GammadetectorPrismaHistograms&);
    bool AllocateHistos() override;
    void Clear() override;
    void Analyze() override;
    bool FillHistos() override;
    void Finalize() override;

  protected:
    Gammadetector&                 gammadetector;
    Prisma&                        prisma;
    GammadetectorPrismaConf&       gammadetectorPrismaConf;
    GammadetectorConf&             gammadetectorConf;
    GammaDC&                       gammaDC;
    GammadetectorPrismaHistograms& gammadetectorPrismaHistograms;
    PrismaAnalysis::PrismaReader&  prismaReader;
};
