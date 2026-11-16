#pragma once
#include "AgataSelector.h"

class UserSelector : public AgataSelector {
  public:
    explicit UserSelector(TFile& oFile) : AgataSelector(oFile){};
    Bool_t Process(Long64_t entry) override;
    void   SlaveBegin(TTree* tree) override;
    void   SlaveTerminate() override;

  private:
    // USER//////////////////////////////////////////////////////
    unsigned long long tsRef  = 0;
    unsigned long long tsCorr = 0;
    unsigned long long tsOld  = 0;
    unsigned long long tsOff  = 0;

    struct UserHistograms {
        std::vector<TObject*> ptrs;
        TDirectory*           dir{nullptr};
        TH1D*                 h_cr_z16            = nullptr;
        TH1D*                 h_cr_z15            = nullptr;
        TH1D*                 h_cr_z14            = nullptr;
        TH1D*                 h_cr_zRatio14over16 = nullptr;

    } userHistograms;
};
