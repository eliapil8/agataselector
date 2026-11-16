#pragma once
#include "AgataSelector.h"

class UserSelector : public AgataSelector {
  public:
    explicit UserSelector(TFile& oFile) : AgataSelector(oFile){};
    Bool_t Process(Long64_t entry) override;
    void   SlaveBegin(TTree* tree) override;
    void   SlaveTerminate() override;

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

  private:
    // USER variable and histogram definition section

    unsigned long long oldTS{0};
    unsigned long long initTS{0};
    int                runNr{-1};
    int                counter{0};

    struct UserHistograms {
        std::vector<TObject*> ptrs;
        TDirectory*           dir{nullptr};
        TH1D*                 h_alive_time = nullptr;
        TH1D*                 h_spiderCR   = nullptr;
    } userHistograms;
};
