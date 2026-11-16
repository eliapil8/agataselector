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
        TH1D*                 Egamma_h59{nullptr};
        TH1D*                 monitor{nullptr};
        TH1D*                 monitor2{nullptr};
        TH1D*                 picco_c{nullptr};
        TH1D*                 fondo{nullptr};
        TH1D*                 fondop{nullptr};
        TH1D*                 fondoa{nullptr};
        TH2D*                 Egamma_Ep_h59{nullptr};
        TH2D*                 Egamma_Eeu{nullptr};
    } userHistograms;
    template <class T, class UT>
    T unsignedDiff(const UT& val1, const UT& val2) {
        UT res{0};
        if(val1 > val2)
            res = val1 - val2;
        else
            res = val2 - val1;

        if(res <= std::numeric_limits<T>::max()) {
            if(val1 > val2)
                return static_cast<T>(res);
            else
                return -static_cast<T>(res);
        }

        throw std::runtime_error("Signed to unsigned conversion problem\n");
    }
};
