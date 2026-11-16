#pragma once
#include "AgataSelector.h"

class UserSelector : public AgataSelector {
  public:
    explicit UserSelector(TFile& oFile) : AgataSelector(oFile){};
    // virtual Bool_t Process(Long64_t entry) override;
    // virtual void   SlaveBegin(TTree* tree) override;
    // virtual void   SlaveTerminate() override;

  private:
    // USER//////////////////////////////////////////////////////
    // struct UserHistograms {
    //     std::vector<TObject*> ptrs;
    //     TDirectory *dir{nullptr};
    // } userHistograms;
};
