#pragma once
#include "AgataSelector.h"

class UserSelector : public AgataSelector {
  public:
    explicit UserSelector(TFile& oFile) : AgataSelector(oFile){};
    Bool_t Process(Long64_t entry) override;
    void   SlaveBegin(TTree* tree) override;
    void   SlaveTerminate() override;

  private:
    // USER variable and histogram definition section

    unsigned long long oldTS{0};
    unsigned long long initTS{0};
    int                runNr{-1};
    int                counter{0};
    double             totCR{0};
    double             aliveT{0};

    struct UserHistograms {
        std::vector<TObject*> ptrs;
        TDirectory*           dir{nullptr};
        TH1D*                 h_alive_time = nullptr;
        TH1D*                 h_avgCR      = nullptr;

        TH1D*                 h_cTime{nullptr};
        TH1D*                 h_noDC_z52{nullptr};
        TH1D*                 h_DC_z52{nullptr};
        TH1D*                 h_DCBP_z52{nullptr};
        TH2D*                 m_Beta_BetaBP_z52{nullptr};
        TH2D*                 m_Theta_ThetaBP_z52{nullptr};
        TH2D*                 m_noDC_noDC_z52{nullptr};
        TH2D*                 m_DC_DC_z52{nullptr};
        TH2D*                 m_DCBP_DCBP_z52{nullptr};
        TH2D*                 m_noDC_theta_z52{nullptr};
        TH2D*                 m_noDC_thetaBP_z52{nullptr};
        TH2D*                 m_DC_theta_z52{nullptr};
        TH2D*                 m_DCBP_theta_z52{nullptr};
        TH2D*                 m_noDC_Qval_z52{nullptr};
        TH2D*                 m_DC_Qval_z52{nullptr};
        TH2D*                 m_DCBP_Qval_z52{nullptr};
        TH2D*                 m_dBeta_EDC_z52{nullptr};
        TH2D*                 m_dTOF_EDC_z52{nullptr};
    } userHistograms;

    template <class T, class UT>
    T unsignedDiff(const UT& val1, const UT& val2) {
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
};
