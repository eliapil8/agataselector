#pragma once

#include "DetectorAnalysis.h"
#include "Prisma.h"
#include "PrismaInputAnalysis.h"
#include <TLine.h>

class PrismaAnalysis : public PrismaInputAnalysis, virtual public DetectorAnalysis
{
  public:
    explicit PrismaAnalysis(Inputs, Prisma &, PrismaInput &);
    void Instantiate() override;
    bool AllocateHistos() override;
    void Analyze() override;
    bool FillHistos() override;
    void Finalize() override;

    double GetUnitsLength() const { return units_length; }
    double GetUnitsAoQ() const { return units_aoq; }
    double GetUnitsTime() const { return units_time; }

    void LoadReactions();
    void FissionCMReconstruction();
    void MaskAcceptanceLimits();

  protected:
    // Container data
    Prisma &prisma;

    CalibrationSelector<2> thetaCMLimits;

    // HistogramStructure
    struct PrismaHistograms : public DetectorHistograms
    {
        struct PrismaRawHistogramStructure : public HistogramStructure
        {
            struct PPAC : public HistogramStructure
            {
                std::array<TH1D *, 10> h_Cathode{nullptr};
                std::array<TH1D *, 10> h_XR{nullptr};
                std::array<TH1D *, 10> h_XL{nullptr};
                std::array<TH1D *, 10> h_XRmXL{nullptr};
                std::array<TH1D *, 10> h_XRmCath{nullptr};
                std::array<TH1D *, 10> h_CathmXL{nullptr};
                std::array<TH2D *, 10> m_XRpXL_CATHODE{nullptr};
                std::array<TH1D *, 10> h_TOF_raw{nullptr};
            } ppac;
            struct MCP : public HistogramStructure
            {
                TH2D *m_MCPX_MCPY{nullptr};
            } mcp;
            struct IC : public HistogramStructure
            {
                std::array<TH1D *, 10> h_IC_A{nullptr};
                std::array<TH1D *, 10> h_IC_B{nullptr};
                std::array<TH1D *, 10> h_IC_C{nullptr};
                std::array<TH1D *, 10> h_IC_D{nullptr};
                std::array<TH1D *, 2>  h_Side_A{nullptr};
                std::array<TH1D *, 2>  h_Side_B{nullptr};
                std::array<TH1D *, 2>  h_Side_C{nullptr};
                std::array<TH1D *, 2>  h_Side_D{nullptr};
                TH2D                  *m_ICNr_ICE{nullptr};
            } ic;
        } raw;

        struct PrismaAnaHistograms : public HistogramStructure
        {
            TH1D *h_ANr{nullptr};
            TH1D *h_ZNr{nullptr};
            TH1D *h_qNr{nullptr};
            TH1D *h_XFP{nullptr};
            TH1D *h_Beta{nullptr};
            TH2D *m_goodEvtsStat{nullptr};
            TH2D *m_ZNr_NNr{nullptr};
            TH2D *m_Zed_Mass{nullptr};
            TH2D *m_Zfloat_Mass{nullptr};
            TH2D *m_Zfloat_Qfloat{nullptr};
            TH2D *m_Qfloat_Mass{nullptr};
            TH2D *m_MCPX_MCPY{nullptr};
            TH2D *m_MCPPhi_MCPTheta{nullptr};
            TH2D *m_XFP_YFP{nullptr};
            TH2D *m_TOF_XFP{nullptr};
            TH2D *m_ICNr_ICE{nullptr};
            TH2D *m_Monitors{nullptr};
            TH2D *m_Brho_Theta{nullptr};

            struct X_FP : public HistogramStructure
            {
                std::array<TH1D *, 10> h_XFP_sections{nullptr};
                std::array<TH1D *, 10> h_XFP_RL_sections{nullptr};
                std::array<TH1D *, 10> h_XFP_RC_sections{nullptr};
                std::array<TH1D *, 10> h_XFP_CL_sections{nullptr};
            } xfp;

            struct DE_E : public HistogramStructure
            {
                struct DEA_E : public HistogramStructure
                {
                    TH2D *m_ICDEA_ICE_tot{nullptr};
                    TH2D *m_ICDEA_ICE_corr_tot{
                        nullptr}; // corrected by angles and if activated by Y_FP and X_FP splines
                    std::array<TH2D *, 10> m_ICDEA_ICE{nullptr};
                } dea_e;
                struct DEAB_E : public HistogramStructure
                {
                    TH2D                  *m_ICDEAB_ICE_tot{nullptr};
                    TH2D                  *m_ICDEAB_ICE_corr_tot{nullptr};
                    std::array<TH2D *, 10> m_ICDEAB_ICE{nullptr};
                } deab_e;
                struct DEA_ERES_CORR : public HistogramStructure
                {
                    TH2D *m_ICDEA_ICEres_tot{nullptr};
                    TH2D *m_ICDEA_ICEres_corr_tot{nullptr};
                    // std::array<TH2D*, 10> m_ICDEA_ICEres_corr{nullptr};
                } dea_eres;
                struct DEAB_ERES_CORR : public HistogramStructure
                {
                    TH2D *m_ICDEAB_ICEres_tot{nullptr};
                    TH2D *m_ICDEAB_ICEres_corr_tot{nullptr};
                } deab_eres;

                // Rotated matrices
                TH2D *m_ICen1_ICen2{nullptr};
                TH2D *m_ICen1_Yfp{nullptr};
                TH2D *m_ICen1_corr1_ICen2{nullptr}; // Corrected by Y_FP
                TH2D *m_ICen1_corr1_Xfp{nullptr};
                TH2D *m_ICen1_corr2_ICen2{nullptr}; // Corrected by X_FP

                TH1D *h_Zed{nullptr};
                TH1D *h_Zfloat{nullptr};
                TH2D *m_Zfloat_ICEres_corr{nullptr};
            } de_e;

            struct ICE_XFP : public HistogramStructure
            {
                TH2D               *m_ICE_XFP_tot{nullptr};
                std::vector<TH2D *> m_ICE_XFP;
            } ice_xfp;

            struct ICE_RBeta : public HistogramStructure
            {
                TH2D               *m_ICE_RBeta_tot{nullptr};
                std::vector<TH2D *> m_ICE_RBeta;

            } ice_rbeta;

            struct ICERBeta_ICE : public HistogramStructure
            {
                TH2D               *m_ICEoverRBeta_ICE_tot{nullptr};
                std::vector<TH2D *> m_ICEoverRBeta_ICE;
            } iceoverrbeta_ice;

            struct Charge : public HistogramStructure
            {
                TH2D *m_Charge_cal_AoQ_tot{nullptr};
                TH2D *m_Charge_cal_AoQ_Z_tot{nullptr};
                TH2D *m_Charge_cal_beta_tot{nullptr};
                TH2D *m_Charge_cal_Xfp_tot{nullptr};
                TH1D *h_Charge_cal_Z_tot{nullptr};
                TH1D *h_Qfloat{nullptr};
                TH2D *m_Qfloat_AoQ{nullptr};
                TH2D *m_Qfloat_beta{nullptr};
            } charge;

            struct Aoverq : public HistogramStructure
            {
                struct ZGated : public HistogramStructure
                {
                    struct Histos : public HistogramStructure
                    {
                        std::vector<TH1D *> h_Aoverq;
                    } histos;
                    struct Matrices : public HistogramStructure
                    {
                        std::vector<TH2D *> m_Aoverq;
                    };
                    Matrices                 matricesXfp;
                    Matrices                 matricesYfp;
                    Matrices                 matricesXmcp;
                    Matrices                 matricesYmcp;
                    std::array<Matrices, 10> matricesTime;
                } zGated;
                struct ZQGated : public HistogramStructure
                {
                    struct Histos : public HistogramStructure
                    {
                        std::vector<std::vector<TH1D *>> h_Aoverq;
                    } histos;
                    struct Matrices : public HistogramStructure
                    {
                        std::vector<std::vector<TH2D *>> m_Aoverq;
                    };
                    Matrices matricesXfp;
                    Matrices matricesYfp;
                    Matrices matricesXmcp;
                    Matrices matricesYmcp;
                    // std::array<Matrices, 10> matricesTime;
                } zqGated;
                TH1D               *h_Aoverq_tot{nullptr};
                std::vector<TH1D *> h_Aoverq_cols;
                TH2D               *m_Aoverq_XFP_tot{nullptr};
                TH2D               *m_Aoverq_Xmcp_tot{nullptr};
                TH2D               *m_Aoverq_Ymcp_tot{nullptr};

            } aoverq;

            struct Mass : public HistogramStructure
            {
                struct ZGated : public HistogramStructure
                {
                    struct Histos : public HistogramStructure
                    {
                        std::vector<TH1D *> h_Mass;
                    } histos;
                    struct Matrices : public HistogramStructure
                    {
                        std::vector<TH2D *> m_Mass_XFP;
                    } matrices;
                } zGated;
                struct ZQGated : public HistogramStructure
                {
                    struct Histos : public HistogramStructure
                    {
                        std::vector<std::vector<TH1D *>> h_Mass;
                    } histos;
                    struct Matrices : public HistogramStructure
                    {
                        std::vector<std::vector<TH2D *>> m_Mass_XFP;
                    } matrices;
                } zqGated;
                TH1D *h_Mass_tot{nullptr};
                TH2D *m_Mass_XFP_tot{nullptr};
            } mass;

            struct TimeEvo : public HistogramStructure
            {
                TH2D *m_TOF_time{nullptr};
                TH2D *m_Aoverq_time{nullptr};
                TH2D *m_Aoverq_z38_time{nullptr};
                TH2D *m_MassIC_time{nullptr};
                TH2D *m_MassTOF_time{nullptr};
                TH2D *m_Charge_time{nullptr};
                TH2D *m_Xfp_time{nullptr};
                TH2D *m_Yfp_time{nullptr};
                TH2D *m_Xmcp_time{nullptr};
                TH2D *m_Xmcp_zoom_time{nullptr};
                TH2D *m_Ymcp_time{nullptr};
                TH2D *m_ICdEA_time{nullptr};
                TH2D *m_ICen1corr2_time{nullptr};
                TH2D *m_ICEres_time{nullptr};
            } timeEvo;

            struct Qvalue : public HistogramStructure
            {
                TH1D *h_Qvalue{nullptr};
                TH2D *m_Qvalue_TKEL{nullptr};

                struct IonHistograms : public HistogramStructure
                {
                    TH1D *h_Qvalue{nullptr};
                };

                std::unordered_map<ReactionFragment::Ion,
                                   IonHistograms,
                                   ReactionFragment::Ion::hash_fn>
                    ionHistos;
            } qvalue;

            struct CMreconstruction : public HistogramStructure
            {
                TH1D *h_betaCM{nullptr};
                TH1D *h_thetaCM{nullptr};
                TH1D *h_phiCM{nullptr};
                TH1D *h_betaCM_98Zr{nullptr};
                TH2D *m_betaCM_thetaCM{nullptr};
                TH2D *m_phiCM_thetaCM{nullptr};
                TH2D *m_betaCM_cosThetaCM_sinThetaCM{nullptr};

                struct RecoIonHistograms : public HistogramStructure
                {
                    TH1D *h_betaCM_ion{nullptr};
                    TH2D *m_BrhoMid_thetaCM_ion{nullptr};
                    TH2D *m_betaCM_thetaCM_ion{nullptr};
                    TH2D *m_betaCM_cosThetaCM_sinThetaCM_ion{nullptr};
                };
                std::unordered_map<ReactionFragment::Ion,
                                   RecoIonHistograms,
                                   ReactionFragment::Ion::hash_fn>
                    recoIonHistos;
            } cmReco;

        } ana;

        struct PrismaMiscHistograms : public HistogramStructure
        {
            std::array<TH2D *, 10> m_XFP_XRmCATHODE{nullptr};
            std::array<TH2D *, 10> m_XFP_CATHODEmXL{nullptr};
            std::array<TH2D *, 10> m_TOFraw_XFP{nullptr};
        } misc;

    } prismaHistograms;

    std::unordered_map<ReactionFragment::Ion, EnergyLoss, ReactionFragment::Ion::hash_fn>
        targetELoss;
    std::unordered_map<ReactionFragment::Ion, EnergyLoss, ReactionFragment::Ion::hash_fn> mcpELoss;

    std::unordered_map<ReactionFragment::Ion,
                       ReactionReconstruction2body<long double>,
                       ReactionFragment::Ion::hash_fn>
        prismaReactions;

    TVector3 betaCM{0, 0, 0};

    double units_length{UNITS::mm};
    double units_aoq{100.};
    double units_time{UNITS::ns};

    TCutG *mask_cut{nullptr};

  private:
    // Readers
    long long EntryNr{0};
};
