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
        TH1D*                 h_LabrID      = nullptr;
    } userHistograms;
};
