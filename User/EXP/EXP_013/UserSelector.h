#pragma once
#include "AgataSelector.h"

class UserSelector : public AgataSelector {
  public:
    explicit UserSelector(TFile& oFile) : AgataSelector(oFile){};
    Bool_t Process(Long64_t entry) override;
    void   SlaveBegin(TTree* tree) override;
    void   SlaveTerminate() override;
    TVector3       GetPrismaPos(double dx = 0., double dy = 0., double dz = 0.,
                                double sx = 0., double sy = 0., double dPhi = 0.,
                                double dTh = 0., double intr_angle = 0.,
                                double intr_thetaX = 0.);

  private:
    // USER variable and histogram definition section

    // unsigned long long timeRef = 0;

    struct UserHistograms {
        std::vector<TObject*> ptrs;
        TDirectory*           dir{nullptr};
        TH2D*                 m_IC_gammaGated    = nullptr;
        TH2D*                 m_EDC_thRot        = nullptr;
        TH2D*                 m_EDC_phiRot       = nullptr;
        TH2D*                 m_EDC_intrAngle    = nullptr;
        TH2D*                 m_EDC_intrThetaX   = nullptr;
        TH2D*                 m_EDC_agataTh      = nullptr;
        TH1D*                 h_TAC_prisma       = nullptr;
        TH1D*                 h_TAC_dante        = nullptr;
        TH2D*                 m_EDC_TAC_prisma   = nullptr;
        TH2D*                 m_E_TAC_prisma     = nullptr;
        TH1D*                 h_EDC_prisma       = nullptr;
        TH1D*                 h_E_prisma         = nullptr;
        TH2D*                 m_d2x_d2y          = nullptr;
        TH2D*                 m_d3x_d3y          = nullptr;
        TH2D*                 m_d3x_d3y_tofgated = nullptr;
        TH2D*                 m_d5x_d5y          = nullptr;
    } userHistograms;

    TCutG* cutZ50 = nullptr;
    TCutG* cutZ48 = nullptr;

    double thMin{-10.};
    double thMax{10.};
};
