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

    // unsigned long long oldTS{0};
    // unsigned long long initTS{0};
    int                runNr{-1};
    unsigned long long counter{0};
    // double             totCR{0};
    // double             activationT{0};
    // double             decayT{0};
    double             timeBin{0.1};
    double             thr{0}; // number in Hz
    // unsigned long long startTime = 0;
    unsigned long long startIrradTime = 0;
    unsigned long long startDecayTime = 0;
    unsigned long long binStartTime   = 0;
    double             minIrradTime{2};
    double             minDecTime{2};
    unsigned long long firstTSofRun = 0;
    bool               decay{false};

    struct UserHistograms {
        std::vector<TObject*> ptrs;
        TDirectory*           dir{nullptr};
        TH1D*                 h_noDC_beam{nullptr};
        TH1D*                 h_noDC_decay{nullptr};
        TH2D*                 m_noDC_noDC_beam{nullptr};
        TH2D*                 m_noDC_noDC_decay{nullptr};
        TH2D*                 m_noDC_decay_time{nullptr};
        TH2D*                 m_noDC_beam_time{nullptr};
        TH1D*                 h_CR{nullptr};
    } userHistograms;
};
