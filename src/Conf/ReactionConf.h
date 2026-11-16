#pragma once

#include <unordered_map>

#include "Conf.h"
#include "ReactionFragment.h"

class ReactionConf : public Conf {
  public:
    ReactionConf();

    double                energy{};               // MeV
    double                reaction_position{0.5}; // percentage of target
    ReactionFragment::Ion beam{};
    ReactionFragment::Ion target{};
    std::unordered_map<ReactionFragment::Ion, bool,
                       ReactionFragment::Ion::hash_fn>
        fragmentsOfInterest{
            {{1, 1}, true}
    };

    // std::unordered_map< std::unordered_set<ReactionFragment::Ion,
    // ReactionFragment::Ion::hash_fn>,
    //                     bool,
    //                     ReactionFragment::Ion::set_hash_fn>
    //                     nBodyFragmentsOfInterest
    //                                     {{{{4,2},{1,1}}, true}};
    std::unordered_map<std::vector<ReactionFragment::Ion>, bool,
                       ReactionFragment::Ion::vec_hash_fn>
        nBodyFragmentsOfInterest{};
};
