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
    struct UserHistograms {
        std::vector<TObject*> ptrs;
        TDirectory*           dir{nullptr};
        TH2D*                 m_spider_vs_agata         = nullptr;
        TH2D*                 m_spider_vs_agata_multip2 = nullptr;
    } userHistograms;
};
