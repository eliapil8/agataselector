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

      std::vector<TH2D*> m_spiderID_EDCBP_gg{};
     
      TH2D* m_spiderID_Theta{nullptr};
      TH2D* m_spiderID_Phi{nullptr};
      TH1D* h_Au{nullptr};
      TH1D* h_Au_down{nullptr};
      TH1D* h_Au_up{nullptr};
      
    } userHistograms;

  ULong64_t firstTS{0};
  
  
};
