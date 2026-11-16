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
        TH1D*                 h_coreE0{nullptr};
        TH1D*                 h_AddE{nullptr};
        TH1D*                 h_trackE{nullptr};
        TH2D*                 m_coreE0_coreE0{nullptr};
        TH2D*                 m_AddE_AddE{nullptr};
        TH2D*                 m_trackE_trackE{nullptr};

    } userHistograms;
};
