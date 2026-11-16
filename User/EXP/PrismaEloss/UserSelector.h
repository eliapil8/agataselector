#pragma once
#include "AgataSelector.h"

class UserSelector : public AgataSelector {
  public:
    explicit UserSelector(const std::string& options)
        : AgataSelector(options){};
    Bool_t Process(Long64_t entry) override;
    void   SlaveBegin(TTree* tree) override;
    void   SlaveTerminate() override;

  class ElossPrisma{
        public:
        static bool FileIsPresent(const std::string&);
        std::unordered_map<ReactionFragment::Ion, EnergyLoss,
                       ReactionFragment::Ion::hash_fn>  MCP_ELoss;
        std::unordered_map<ReactionFragment::Ion, EnergyLoss,
                          ReactionFragment::Ion::hash_fn>  PPAC_gas_ELoss;
        std::unordered_map<ReactionFragment::Ion, EnergyLoss,
                            ReactionFragment::Ion::hash_fn>  PPAC_window_ELoss;
        std::unordered_map<ReactionFragment::Ion, EnergyLoss,
                            ReactionFragment::Ion::hash_fn>  IC_gas_ELoss;                        
    } elossPrisma;

  private:
    // USER variable and histogram definition section

    // unsigned long long timeRef = 0;

    struct UserHistograms {
        std::vector<TObject*> ptrs;
        TDirectory*           dir{nullptr};

    } userHistograms;
};
