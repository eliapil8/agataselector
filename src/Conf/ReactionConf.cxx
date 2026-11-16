#include "ReactionConf.h"

ReactionConf::ReactionConf() : Conf("REACTION_CONF") {
    parD.emplace("ENERGY",
                 Conf::Property<double&>{energy, "MeV", "Beam energy"});
    parD.emplace(
        "REACTION_POSITION",
        Conf::Property<double&>{reaction_position, "#",
                                "Position of the reaction in the taget "
                                "0->front 0.5->middle, 1->back"});

    parION.emplace("BEAM", Conf::Property<ReactionFragment::Ion&>{
                               beam, "#", "Beam ion A Z"});
    parION.emplace("TARGET", Conf::Property<ReactionFragment::Ion&>{
                                 target, "#", "Target ion A Z"});

    parFI.emplace(
        "ION",
        Conf::Property<std::unordered_map<ReactionFragment::Ion, bool,
                                          ReactionFragment::Ion::hash_fn>&>{
            fragmentsOfInterest, "#",
            "Fragment of interest for binary reaction calculation: A Z (those "
            "detected)"});

    parAFI.emplace(
        "IONS",
        Property<std::unordered_map<std::vector<ReactionFragment::Ion>, bool,
                                    ReactionFragment::Ion::vec_hash_fn>&>{
            nBodyFragmentsOfInterest, "#",
            "Fragment of interest for binary reaction calculation: A Z (those "
            "detected)"});
}
