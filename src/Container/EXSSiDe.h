#pragma once

#include <TLorentzVector.h>

#include <iostream>
#include <string>

#include "Container.h"

class EXSSiDe : public Container {
  public:
    unsigned int                IdPside;           // ID
    unsigned int                IdNside;           // ID
    unsigned int                A;                 // ID A
    unsigned int                Z;                 // ID Z
    bool                        identified{false}; // Writing gates
    bool                        good{false}; // Writing gates
    std::pair<double, double>   E;   // Energy considering energy losses
    double                      ESi{0}; // Calibrated energy summed P-N coincident events
    //std::vector<double>         Ex;  // Excitation energy
    TVector3                    Pos; // Hit position
    TLorentzVector              p4;  // 4 momentum of recoil
    TLorentzVector              p3;  // 4 momentum of recoil
    void                        resize(const unsigned int& mult) override {
        Container::resize(mult);
        }
    void clear() override {
        Container::clear();
        ESi = 0.;
        Pos.SetXYZ(9999,9999,9999);
        E = {};
        identified = false;
        good = false;
    }
    EXSSiDe() = default;
    ClassDefOverride(EXSSiDe, 2);
};
