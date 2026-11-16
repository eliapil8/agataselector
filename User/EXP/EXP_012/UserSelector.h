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

    // unsigned long long timeRef = 0;

    struct UserHistograms {
        std::vector<TObject*> ptrs;
        TDirectory*           dir{nullptr};
        TH2D*                 m_EDC_crystalID = nullptr;

    } userHistograms;
};
