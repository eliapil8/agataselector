#pragma once
#include "AgataSelector.h"

template <class T, class UT> T unsignedDiff(const UT& val1, const UT& val2) {
    UT res{0};
    if(val1 > val2)
        res = val1 - val2;
    else
        res = val2 - val1;

    if(res <= std::numeric_limits<T>::max()) {
        if(val1 > val2)
            return static_cast<T>(res);
        else
            return -static_cast<T>(res);
    }

    throw std::runtime_error("Signed to unsigned conversion problem\n");
}
template <class T, class UT> T signedToUnsigned(const UT& val) {
    if(val <= std::numeric_limits<T>::max()) return static_cast<T>(val);

    throw std::runtime_error("Signed to unsigned conversion problem\n");
}

class UserSelector : public AgataSelector {
  public:
    UserSelector(const std::string& options) : AgataSelector(options){};
    virtual Bool_t Process(Long64_t entry) override;
    virtual void   SlaveBegin(TTree* tree) override;
    virtual void   SlaveTerminate() override;

  private:
    // USER//////////////////////////////////////////////////////

    unsigned long long initTS{0};

    struct Histograms {
        std::vector<TObject*> ptrs;
        TDirectory*           dir{nullptr};
    };

    struct UserHistograms : public Histograms {
        class Coincidence {
          public:
            Coincidence(std::string& name, double elow1, double ehigh1,
                        double elow2, double ehigh2, double leftshift1,
                        double rightshift1, double leftshift2,
                        double rightshift2);
            ~Coincidence();

            TDirectory*        dir{nullptr};
            std::string        Name;
            double             Elow1;
            double             Ehigh1;
            double             Egamma1;
            double             Elow2;
            double             Ehigh2;
            double             Egamma2;
            double             GateWidth1; // Gating interval on gamma above
            double             GateWidth2;
            double             LeftShift1{0};
            double             RightShift1{0};
            double             LeftShift2{0};
            double             RightShift2{0};

            unsigned long long InitTS{0};

            struct AngCorrHistograms : public Histograms {
                TH1D* h_singles{nullptr};
                TH2D* h_gamma_gamma{nullptr};
                TH2D* h_id_vs_time_total{nullptr};
                TH2D* h_id_vs_time_gated{nullptr};
                TH1D* h_gg_Theta{nullptr};
                TH1D* h_gg_ang_corr{nullptr};
                TH1D* h_gg_Theta_uncorrelated{nullptr};
                TH1D* h_gg_Theta_bg{nullptr};
                TH1D* h_gg_Theta_bg_uncorrelated{nullptr};
                TH1D* h_gg_Theta_random{nullptr};
                TH1D* h_gg_Theta_random_uncorrelated{nullptr};
                TH1D* h_gg_Theta_corrected{nullptr};
            };

            bool CrysSpectra{false};
            struct CrysHistograms : public Histograms {
                TH1D* h_core_0{nullptr};
                TH1D* h_core_1{nullptr};
                TH1D* h_core_2{nullptr};
                TH1D* h_core_3{nullptr};
                TH1D* h_core_4{nullptr};
                TH1D* h_core_5{nullptr};
                TH1D* h_core_6{nullptr};
                TH1D* h_core_7{nullptr};
                TH1D* h_core_8{nullptr};
                TH1D* h_core_9{nullptr};
                TH1D* h_core_10{nullptr};
                TH1D* h_core_11{nullptr};
                TH1D* h_core_12{nullptr};
                TH1D* h_core_13{nullptr};
                TH1D* h_core_14{nullptr};
                TH1D* h_core_15{nullptr};
                TH1D* h_core_16{nullptr};
                TH1D* h_core_17{nullptr};
                TH1D* h_core_18{nullptr};
                TH1D* h_core_19{nullptr};
                TH1D* h_core_20{nullptr};
                TH1D* h_core_21{nullptr};
                TH1D* h_core_22{nullptr};
                TH1D* h_core_23{nullptr};
                TH1D* h_core_24{nullptr};
                TH1D* h_core_25{nullptr};
                TH1D* h_core_26{nullptr};
                TH1D* h_core_27{nullptr};
                TH1D* h_core_28{nullptr};
                TH1D* h_core_29{nullptr};
                TH1D* h_core_30{nullptr};
                TH1D* h_core_31{nullptr};
                TH1D* h_core_32{nullptr};
                TH1D* h_core_33{nullptr};
                TH1D* h_core_34{nullptr};
                TH1D* h_core_35{nullptr};
                TH1D* h_core_36{nullptr};
                TH1D* h_core_37{nullptr};
                TH1D* h_core_38{nullptr};
                TH1D* h_core_39{nullptr};
                TH1D* h_core_40{nullptr};
                TH1D* h_core_41{nullptr};
            } crysHistos;

            AngCorrHistograms     CoreHisto;
            AngCorrHistograms     TrackHisto;
            AngCorrHistograms     AddHisto;
            std::vector<TVector3> core_vec_pos_1;
            std::vector<TVector3> core_vec_pos_2;
            std::vector<TVector3> tracked_vec_pos_1;
            std::vector<TVector3> tracked_vec_pos_2;
            std::vector<TVector3> addback_vec_pos_1;
            std::vector<TVector3> addback_vec_pos_2;
            std::vector<TVector3> bg_core_vec_pos_1;
            std::vector<TVector3> bg_core_vec_pos_2;
            std::vector<TVector3> bg_tracked_vec_pos_1;
            std::vector<TVector3> bg_tracked_vec_pos_2;
            std::vector<TVector3> bg_addback_vec_pos_1;
            std::vector<TVector3> bg_addback_vec_pos_2;
            std::vector<TVector3> random_core_vec_pos_1;
            std::vector<TVector3> random_core_vec_pos_2;

            TF1*                  ang_corr_func_core;
            TF1*                  ang_corr_func_tracked;
            TF1*                  ang_corr_func_addback;

            void                  Initialize();
            void                  SetInitTS(unsigned long long initTS) {
                InitTS = initTS;
                std::cout << "Init TS   " << InitTS << std::endl;
            };
            unsigned long long GetInitTS() { return InitTS; };
            void SetCrysSpectraOn(bool answer) { CrysSpectra = answer; };
            void Compute(AgataAnalysis::AgataReader& agataReader);
            void UncorrProcess();

            // void FitDistribution(){
            //
            //     ang_corr_func_core = new TF1("core_func","Fitted angular
            //     correlation
            //     core","[0]*(1+ROOT::Math::legendre(2,x)*0.1020+ROOT::Math::legendre(4,x)*0.0091)",0,180);
            //     ang_corr_func_tracked = new TF1("tracked_func","Fitted
            //     angular correlation
            //     tracked","[0]*(1+ROOT::Math::legendre(2,x)*0.1020+ROOT::Math::legendre(4,x)*0.0091)",0,180);
            //     ang_corr_func_addback = new TF1("addback_func","Fitted
            //     angular correlation
            //     addback","[0]*(1+ROOT::Math::legendre(2,x)*0.1020+ROOT::Math::legendre(4,x)*0.0091)",0,180);
            //
            //
            //     CoreHisto.h_gg_ang_corr->Fit(ang_corr_func_core,"RIE");
            //     TrackHisto.h_gg_ang_corr->Fit(ang_corr_func_tracked,"IE",30,180);
            //     AddHisto.h_gg_ang_corr->Fit(ang_corr_func_addback,"IE",30,180);
            //
            //
            //
            //     return;
            // };

          private:
        };

        std::vector<Coincidence> coincs;

    } userHistograms;
};
