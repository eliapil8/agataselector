#pragma once

#include <iostream>
#include <vector>

#include <TLorentzVector.h>
#include <TVector3.h>

#include "ReactionFragment.h"
#include "Container.h"

class Dante : public Container {
  public:
    enum TofType { PRISMA, DANTE, NONE };
    std::vector<std::pair<double, TofType>>
                                TOF; // Cfd time difference with respect to TS
    std::vector<double>         ex;  // exc energy
	 std::vector<double>         Qval;  // 4 momentum of other fragment
    std::vector<TLorentzVector> p4;  // 4 momentum of other fragment
    std::vector<TLorentzVector> p3;  // 4 momentum of other fragment
    std::vector<TVector3>       Pos; // Position of detection
    std::vector<unsigned int>   Id;  // ID
    std::vector<int>   			  matchingDanteIndex;  // ID of the matching dante
    std::vector<int>            TOFregion;  //to indicate if low or high TOF 
    std::vector<ReactionFragment::Ion> ions; // ions associated with given "hit"
    bool is_tof_assignemnt_valid;
    void                        resize(const unsigned int& mult) override {
        Container::resize(mult);
        TOF.resize(mult, {-100., NONE});
        ex.resize(mult, 0);
		  Qval.resize(mult, -1000);
        p4.resize(mult);
        p3.resize(mult);
        Pos.resize(mult);
        Id.resize(mult);
        ions.resize(mult);
        matchingDanteIndex.resize(mult, -1);    
		  TOFregion.resize(mult, -1); 
        is_tof_assignemnt_valid = false;
    }

    void clear() override {
        Container::clear();
        TOF.clear();
		  Qval.clear();
        ex.clear();
        p4.clear();
        p3.clear();
        Pos.clear();
        Id.clear();
        ions.clear();
        matchingDanteIndex.clear();
        TOFregion.clear();
		  is_tof_assignemnt_valid = false;
    }
    Dante() = default;
    ClassDefOverride(Dante, 3);
};
