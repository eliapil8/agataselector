#pragma once
#include "AgataSelector.h"

class UserSelector : public AgataSelector {
  public:
    UserSelector(const std::string& options) : AgataSelector(options){};
    virtual Bool_t Process(Long64_t entry) override;
    virtual void   SlaveBegin(TTree* tree) override;
    virtual void   SlaveTerminate() override;

  private:
    // USER//////////////////////////////////////////////////////
    struct UserHistograms {
        std::vector<TObject*> ptrs;
        TDirectory*           dir{nullptr};
        TH1D*                 h_Beta_Prisma_z17{nullptr};
        TH1D*                 h_TSdiff_z17_561keV{nullptr};
        TH2D*                 m_EDC_Theta_Z17_A35{nullptr};
        TH2D*                 m_EDCBP_Theta_Z17_A35{nullptr};
        TH2D*                 m_Eg_Theta_Z17_A35{nullptr};
        TH2D*                 m_EBP_Theta_Z17_A35{nullptr};

        TH2D*                 m_Theta_p3p4_z17{nullptr};
        TH2D*                 m_Theta_Beta_p3_z17{nullptr};
        TH2D*                 m_Theta_Beta_p4_z17{nullptr};
        TH2D*                 m_TKE_E_Z17_A35{nullptr};
        TH2D*                 m_TKE_EDC_Z17_A35{nullptr};
        TH2D*                 m_TKE_EDCBP_Z17_A35{nullptr};
        TH2D*                 m_MCPXY_z17{nullptr};

        TH2D*                 m_EDC_Theta_Z15_A34{nullptr};
        TH2D*                 m_EDCBP_Theta_Z15_A34{nullptr};
        TH2D*                 m_Eg_Theta_Z15_A34{nullptr};
        TH2D*                 m_EBP_Theta_Z15_A34{nullptr};

        TH2D*                 m_Theta_p3p4_z15{nullptr};
        TH2D*                 m_Theta_Beta_p3_z15{nullptr};
        TH2D*                 m_Theta_Beta_p4_z15{nullptr};
        TH2D*                 m_TKE_E_Z15_A34{nullptr};
        TH2D*                 m_TKE_EDC_Z15_A34{nullptr};
        TH2D*                 m_TKE_EDCBP_Z15_A34{nullptr};

    } userHistograms;
};
