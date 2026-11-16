#pragma once
#include "AgataSelector.h"

class UserSelector : public AgataSelector {
  public:
    UserSelector(const std::string& options) : AgataSelector(options){};
    virtual Bool_t Process(Long64_t entry) override;
    virtual void   SlaveBegin(TTree* tree) override;
    virtual void   SlaveTerminate() override;
    TVector2       ProjectionPointOnLine(TVector2 point, double m, double c);

    unsigned long long oldTS{0};
    unsigned long long initTS{0};
    int                runNr{-1};
    int                counter{0};
    double             totCR{0};
    double             aliveT{0};

  private:
    // USER//////////////////////////////////////////////////////

    class UserPrismaAnalysis : public DetectorAnalysis {
      public:
        explicit UserPrismaAnalysis(Inputs, Prisma&);
        void Instantiate() override;
        bool AllocateHistos() override;
        bool Clear() override;
        bool Analyze() override;
        bool FillHistos() override;
        bool Finalize() override;

        struct AoQ2D {
            struct AOQvsXMCP : public HistogramStructure {
                struct ZGated : public HistogramStructure {
                    std::vector<TH2D*> m_AoQvsXmcp;
                } zGated;
            } aoqvsxmcp;
            struct AOQvsYMCP : public HistogramStructure {
                struct ZGated : public HistogramStructure {
                    std::vector<TH2D*> m_AoQvsYmcp;
                } zGated;
            } aoqvsymcp;
            struct AOQvsXFP : public HistogramStructure {
                struct ZGated : public HistogramStructure {
                    std::vector<TH2D*> m_AoQvsXfp;
                } zGated;
            } aoqvsxfp;
        } prismaAoq;

        struct PrismaRuns : public HistogramStructure {
            TH2D* m_MCP_X_vs_runNr{nullptr};
            TH2D* m_MCP_Y_vs_runNr{nullptr};
            TH2D* m_MCP_Theta_vs_runNr{nullptr};
            TH2D* m_MCP_Phi_vs_runNr{nullptr};
            TH2D* m_X_FP_vs_runNr{nullptr};
            TH2D* m_Y_FP_vs_runNr{nullptr};
            TH2D* m_TOF_vs_runNr{nullptr};
            TH2D* m_IC_E_vs_runNr{nullptr};
            TH2D* m_IC_DE_A_vs_runNr{nullptr};
            TH2D* m_IC_DE_AB_vs_runNr{nullptr};
            TH2D* m_IC_RANGE_vs_runNr{nullptr};
            TH2D* m_IC_Drift_A_vs_runNr{nullptr};
            TH2D* m_IC_Drift_B_vs_runNr{nullptr};
            TH2D* m_IC_Drift_C_vs_runNr{nullptr};
            TH2D* m_Beta_vs_runNr{nullptr};
            TH2D* m_Radius_vs_runNr{nullptr};
            TH2D* m_RBeta_vs_runNr{nullptr};
            TH2D* m_A_over_q_vs_runNr{nullptr};
            TH2D* m_Mass_vs_runNr{nullptr};
            TH2D* m_Qvalue_vs_runNr{nullptr};
            TH2D* m_Z_Nr_vs_runNr{nullptr};
            TH2D* m_Q_Nr_vs_runNr{nullptr};
            TH2D* m_A_Nr_vs_runNr{nullptr};
            TH2D* m_Mass_z36_vs_runNr{nullptr};
            TH2D* m_Mass_z34_vs_runNr{nullptr};
            TH2D* m_Mass_z32_vs_runNr{nullptr};
            TH2D* m_Mass_z30_vs_runNr{nullptr};
            TH2D* m_TOF_z36_vs_runNr{nullptr};
            TH2D* m_TOF_z34_vs_runNr{nullptr};
            TH2D* m_TOF_z32_vs_runNr{nullptr};
            TH2D* m_TOF_z30_vs_runNr{nullptr};
        } hprismaruns;

    } userPrisma;

    class UserAgataAnalysis : public DetectorAnalysis {
      public:
        UserAgataAnalysis();

        bool AllocateHistos();
        bool Clear();
        bool Analyze();
        bool FillHistos();
        bool Finalize();

        struct AgataTime : public HistogramStructure {
            TH1D* h_CR{nullptr};
            TH2D* m_EnoDC_vs_TS{nullptr};
            TH2D* m_CrysId_vs_TS{nullptr};
            TH2D* m_trackE_00A_vs_TS{nullptr};
            TH2D* m_trackE_00B_vs_TS{nullptr};
            TH2D* m_trackE_00C_vs_TS{nullptr};
            TH2D* m_trackE_01A_vs_TS{nullptr};
            TH2D* m_trackE_01B_vs_TS{nullptr};
            TH2D* m_trackE_01C_vs_TS{nullptr};
            TH2D* m_trackE_02A_vs_TS{nullptr};
            TH2D* m_trackE_02B_vs_TS{nullptr};
            TH2D* m_trackE_02C_vs_TS{nullptr};
            TH2D* m_trackE_04A_vs_TS{nullptr};
            TH2D* m_trackE_04B_vs_TS{nullptr};
            TH2D* m_trackE_04C_vs_TS{nullptr};
            TH2D* m_trackE_05A_vs_TS{nullptr};
            TH2D* m_trackE_05B_vs_TS{nullptr};
            TH2D* m_trackE_05C_vs_TS{nullptr};
            TH2D* m_trackE_06A_vs_TS{nullptr};
            TH2D* m_trackE_06B_vs_TS{nullptr};
            TH2D* m_trackE_06C_vs_TS{nullptr};
            TH2D* m_trackE_07A_vs_TS{nullptr};
            TH2D* m_trackE_07B_vs_TS{nullptr};
            TH2D* m_trackE_07C_vs_TS{nullptr};
            TH2D* m_trackE_08A_vs_TS{nullptr};
            TH2D* m_trackE_08B_vs_TS{nullptr};
            TH2D* m_trackE_08C_vs_TS{nullptr};
            TH2D* m_trackE_09A_vs_TS{nullptr};
            TH2D* m_trackE_09B_vs_TS{nullptr};
            TH2D* m_trackE_09C_vs_TS{nullptr};
            TH2D* m_trackE_10A_vs_TS{nullptr};
            TH2D* m_trackE_10B_vs_TS{nullptr};
            TH2D* m_trackE_10C_vs_TS{nullptr};
            TH2D* m_trackE_11A_vs_TS{nullptr};
            TH2D* m_trackE_11B_vs_TS{nullptr};
            TH2D* m_trackE_11C_vs_TS{nullptr};
            TH2D* m_trackE_13A_vs_TS{nullptr};
            TH2D* m_trackE_13B_vs_TS{nullptr};
            TH2D* m_trackE_13C_vs_TS{nullptr};
            TH2D* m_trackE_14A_vs_TS{nullptr};
            TH2D* m_trackE_14B_vs_TS{nullptr};
            TH2D* m_trackE_14C_vs_TS{nullptr};
            std::unordered_map<unsigned int, TH2D*> m_trackE_Crys_vs_TS;
        } hagatatime;

        struct AgataMix : public HistogramStructure {
            TH2D* m_EnoDC_vs_coreID{nullptr};
        } hagatamix;

        struct AgataRuns : public Histograms {
            TH2D* m_EnoDC_vs_runNr{nullptr};
            TH2D* m_CR_vs_runNr{nullptr};
            TH2D* m_CrysId_vs_runNr{nullptr};
            TH1D* h_trackE{nullptr};
        } hagataruns;

    } userAgata;

    class UserAgataPrismaAnalysis : public CoincidenceAnalysis {
      public:
        UserAgataPrismaAnalysis();

        bool AllocateHistos();
        bool Clear();
        bool Analyze();
        bool FillHistos();
        bool Finalize();

        struct AgataPrismaRuns : public HistogramStructure {
            TH2D* m_Tdiff_PrismaAgata_vs_runNr{nullptr};
            TH2D* m_EDC_86Kr_vs_runNr{nullptr};
            TH2D* m_EDC_88Kr_vs_runNr{nullptr};
            TH2D* m_EDC_84Se_vs_runNr{nullptr};
            TH2D* m_EDC_90Se_vs_runNr{nullptr};
            TH2D* m_EDC_78Ge_vs_runNr{nullptr};
            TH2D* m_EDC_82Ge_vs_runNr{nullptr};
            TH2D* m_EDC_76Zn_vs_runNr{nullptr};
            TH2D* m_EDC_78Zn_vs_runNr{nullptr};
            TH2D* m_EDC_79Zn_vs_runNr{nullptr};
            TH2D* m_EDC_80Zn_vs_runNr{nullptr};
        } hagataprismaruns;

        struct AgataPrismaMix : public HistogramStructure {
            struct IonHistograms : public Histograms {
                TH1D* h_ion_EDC_high_energy{nullptr};
                TH1D* h_ion_EnoDC_high_energy{nullptr};
                TH2D* m_ion_EDC_vs_coreID{nullptr};
                TH2D* m_ion_EnoDC_vs_coreID{nullptr};
                TH2D* m_ion_EDC_vs_Beta{nullptr};
                TH2D* m_ion_EDC_vs_AGATA_zShift{nullptr};
                TH2D* m_ion_EDC_vs_AGATA_phi{nullptr};
                TH2D* m_ion_EDC_vs_MCP_zShift{nullptr};
                TH2D* m_ion_EDC_vs_MCP_xScale{nullptr};
                TH2D* m_ion_EDC_vs_MCP_yScale{nullptr};
            };
            std::unordered_map<ReactionFragment::Ion, IonHistograms,
                               ReactionFragment::Ion::hash_fn>
                ionHistos;

        } hagataprismamix;

    } userAgata;
    /*
        struct UserHistograms {
            std::vector<TObject*> ptrs;
            TDirectory *dir{nullptr};
            struct AOQvsXMCP : public Histograms {
                struct ZGated : public Histograms {
                    std::vector<TH2D*> m_AoQvsXmcp;
                } zGated;
            } aoqvsxmcp;
            struct AOQvsYMCP : public Histograms {
                struct ZGated : public Histograms {
                    std::vector<TH2D*> m_AoQvsYmcp;
                } zGated;
            } aoqvsymcp;
            struct AOQvsXFP : public Histograms {
                struct ZGated : public Histograms {
                    std::vector<TH2D*> m_AoQvsXfp;
                } zGated;
            } aoqvsxfp;

            struct histoRuns : public Histograms
            {

                public:
                    struct AgataRuns : public Histograms
                    {
                        TH2D * m_EnoDC_vs_runNr{nullptr};
                        TH2D * m_CR_vs_runNr{nullptr};
                        TH2D * m_CrysId_vs_runNr{nullptr};
                        TH1D * h_trackE{nullptr};
                    } hagataruns;
                    struct PrismaRuns : public Histograms
                    {
                        TH2D * m_MCP_X_vs_runNr{nullptr};
                        TH2D * m_MCP_Y_vs_runNr{nullptr};
                        TH2D * m_MCP_Theta_vs_runNr{nullptr};
                        TH2D * m_MCP_Phi_vs_runNr{nullptr};
                        TH2D * m_X_FP_vs_runNr{nullptr};
                        TH2D * m_Y_FP_vs_runNr{nullptr};
                        TH2D * m_TOF_vs_runNr{nullptr};
                        TH2D * m_IC_E_vs_runNr{nullptr};
                        TH2D * m_IC_DE_A_vs_runNr{nullptr};
                        TH2D * m_IC_DE_AB_vs_runNr{nullptr};
                        TH2D * m_IC_RANGE_vs_runNr{nullptr};
                        TH2D * m_IC_Drift_A_vs_runNr{nullptr};
                        TH2D * m_IC_Drift_B_vs_runNr{nullptr};
                        TH2D * m_IC_Drift_C_vs_runNr{nullptr};
                        TH2D * m_Beta_vs_runNr{nullptr};
                        TH2D * m_Radius_vs_runNr{nullptr};
                        TH2D * m_RBeta_vs_runNr{nullptr};
                        TH2D * m_A_over_q_vs_runNr{nullptr};
                        TH2D * m_Mass_vs_runNr{nullptr};
                        TH2D * m_Qvalue_vs_runNr{nullptr};
                        TH2D * m_Z_Nr_vs_runNr{nullptr};
                        TH2D * m_Q_Nr_vs_runNr{nullptr};
                        TH2D * m_A_Nr_vs_runNr{nullptr};
                        TH2D * m_Mass_z36_vs_runNr{nullptr};
                        TH2D * m_Mass_z34_vs_runNr{nullptr};
                        TH2D * m_Mass_z32_vs_runNr{nullptr};
                        TH2D * m_Mass_z30_vs_runNr{nullptr};
                        TH2D * m_TOF_z36_vs_runNr{nullptr};
                        TH2D * m_TOF_z34_vs_runNr{nullptr};
                        TH2D * m_TOF_z32_vs_runNr{nullptr};
                        TH2D * m_TOF_z30_vs_runNr{nullptr};
                    } hprismaruns;

                    struct AgataPrismaRuns : public Histograms
                    {
                        TH2D * m_Tdiff_PrismaAgata_vs_runNr{nullptr};
                        TH2D * m_EDC_86Kr_vs_runNr{nullptr};
                        TH2D * m_EDC_88Kr_vs_runNr{nullptr};
                        TH2D * m_EDC_84Se_vs_runNr{nullptr};
                        TH2D * m_EDC_90Se_vs_runNr{nullptr};
                        TH2D * m_EDC_78Ge_vs_runNr{nullptr};
                        TH2D * m_EDC_82Ge_vs_runNr{nullptr};
                        TH2D * m_EDC_76Zn_vs_runNr{nullptr};
                        TH2D * m_EDC_78Zn_vs_runNr{nullptr};
                        TH2D * m_EDC_79Zn_vs_runNr{nullptr};
                        TH2D * m_EDC_80Zn_vs_runNr{nullptr};
                    } hagataprismaruns;

                    struct AgataPrismaRuns : public Histograms {
                        TH2D* m_Tdiff_PrismaAgata_vs_runNr{nullptr};
                        TH2D* m_EDC_86Kr_vs_runNr{nullptr};
                        TH2D* m_EDC_88Kr_vs_runNr{nullptr};
                        TH2D* m_EDC_84Se_vs_runNr{nullptr};
                        TH2D* m_EDC_90Se_vs_runNr{nullptr};
                        TH2D* m_EDC_78Ge_vs_runNr{nullptr};
                        TH2D* m_EDC_82Ge_vs_runNr{nullptr};
                        TH2D* m_EDC_76Zn_vs_runNr{nullptr};
                        TH2D* m_EDC_78Zn_vs_runNr{nullptr};
                        TH2D* m_EDC_79Zn_vs_runNr{nullptr};
                        TH2D* m_EDC_80Zn_vs_runNr{nullptr};
                    } hagataprismaruns;

            } historuns;

            struct histoTime : public Histograms
            {
                struct AgataTime : public Histograms
                    {
                        TH1D * h_CR{nullptr};
                        TH2D * m_EnoDC_vs_TS{nullptr};
                        TH2D * m_CrysId_vs_TS{nullptr};
                        TH2D * m_trackE_00A_vs_TS{nullptr};
                        TH2D * m_trackE_00B_vs_TS{nullptr};
                        TH2D * m_trackE_00C_vs_TS{nullptr};
                        TH2D * m_trackE_01A_vs_TS{nullptr};
                        TH2D * m_trackE_01B_vs_TS{nullptr};
                        TH2D * m_trackE_01C_vs_TS{nullptr};
                        TH2D * m_trackE_02A_vs_TS{nullptr};
                        TH2D * m_trackE_02B_vs_TS{nullptr};
                        TH2D * m_trackE_02C_vs_TS{nullptr};
                        TH2D * m_trackE_04A_vs_TS{nullptr};
                        TH2D * m_trackE_04B_vs_TS{nullptr};
                        TH2D * m_trackE_04C_vs_TS{nullptr};
                        TH2D * m_trackE_05A_vs_TS{nullptr};
                        TH2D * m_trackE_05B_vs_TS{nullptr};
                        TH2D * m_trackE_05C_vs_TS{nullptr};
                        TH2D * m_trackE_06A_vs_TS{nullptr};
                        TH2D * m_trackE_06B_vs_TS{nullptr};
                        TH2D * m_trackE_06C_vs_TS{nullptr};
                        TH2D * m_trackE_07A_vs_TS{nullptr};
                        TH2D * m_trackE_07B_vs_TS{nullptr};
                        TH2D * m_trackE_07C_vs_TS{nullptr};
                        TH2D * m_trackE_08A_vs_TS{nullptr};
                        TH2D * m_trackE_08B_vs_TS{nullptr};
                        TH2D * m_trackE_08C_vs_TS{nullptr};
                        TH2D * m_trackE_09A_vs_TS{nullptr};
                        TH2D * m_trackE_09B_vs_TS{nullptr};
                        TH2D * m_trackE_09C_vs_TS{nullptr};
                        TH2D * m_trackE_10A_vs_TS{nullptr};
                        TH2D * m_trackE_10B_vs_TS{nullptr};
                        TH2D * m_trackE_10C_vs_TS{nullptr};
                        TH2D * m_trackE_11A_vs_TS{nullptr};
                        TH2D * m_trackE_11B_vs_TS{nullptr};
                        TH2D * m_trackE_11C_vs_TS{nullptr};
                        TH2D * m_trackE_13A_vs_TS{nullptr};
                        TH2D * m_trackE_13B_vs_TS{nullptr};
                        TH2D * m_trackE_13C_vs_TS{nullptr};
                        TH2D * m_trackE_14A_vs_TS{nullptr};
                        TH2D * m_trackE_14B_vs_TS{nullptr};
                        TH2D * m_trackE_14C_vs_TS{nullptr};
                        std::unordered_map <unsigned int, TH2D*>
       m_trackE_Crys_vs_TS; } hagatatime;

            } histotime;

            struct histoMix : public Histograms
            {
                TH2D * m_EnoDC_vs_coreID{nullptr};
                TH1D * h_Z{nullptr};

                struct IonHistograms : public Histograms{
                    TH1D * h_ion_EDC_high_energy{nullptr};
                    TH1D * h_ion_EnoDC_high_energy{nullptr};
                    TH2D * m_ion_EDC_vs_coreID{nullptr};
                    TH2D * m_ion_EnoDC_vs_coreID{nullptr};
                    TH2D * m_ion_EDC_vs_Beta{nullptr};
                    TH2D * m_ion_EDC_vs_AGATA_zShift{nullptr};
                    TH2D * m_ion_EDC_vs_AGATA_phi{nullptr};
                    TH2D * m_ion_EDC_vs_MCP_zShift{nullptr};
                    TH2D * m_ion_EDC_vs_MCP_xScale{nullptr};
                    TH2D * m_ion_EDC_vs_MCP_yScale{nullptr};

                };
                std::unordered_map<ReactionFragment::Ion,
       IonHistograms,ReactionFragment::Ion::hash_fn> ionHistos;

            } histomix;



        } userhistograms;

        */
};
