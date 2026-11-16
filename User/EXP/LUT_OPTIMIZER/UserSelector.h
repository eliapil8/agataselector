#pragma once
#include "AgataSelector.h"

class UserSelector : public AgataSelector {
  public:
    explicit UserSelector(const std::string& options)
        : AgataSelector(options){};
    Bool_t Process(Long64_t entry) override;
    void   SlaveBegin(TTree* tree) override;
    void   SlaveTerminate() override;

  private:
    // USER variable and histogram definition section

    // unsigned long long timeRef = 0;

    struct UserHistograms {
        std::vector<TObject*> ptrs;
        TDirectory *dir{nullptr};
        std::map<std::string, std::vector<TH1D*>> histos;
    } userHistograms;
};
