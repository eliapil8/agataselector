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

    struct Histograms {
        std::vector<TObject*> ptrs;
        TDirectory*           dir{nullptr};
    };

    struct UserHistograms : public Histograms {

      struct IonHistograms : public Histograms {
	TH1D *h_DC{nullptr};
	TH2D *m_DC_Theta{nullptr};
	TH2D *m_DC_Qvalue{nullptr};
	TH2D *m_DC_DC{nullptr};
      };
      std::unordered_map<ReactionFragment::Ion, IonHistograms, ReactionFragment::Ion::hash_fn> ionHistos;
      
    } userHistograms;
    
};
