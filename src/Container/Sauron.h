#pragma once

#include <TLorentzVector.h>

#include <iostream>
#include <string>

#include "Container.h"

class Sauron : public Container {
  public:
    std::vector<unsigned int>   IdPside;           // ID
    std::vector<unsigned int>   IdNside;           // ID
    std::vector<unsigned int>   A;                 // ID A
    std::vector<unsigned int>   Z;                 // ID Z
    std::vector<bool>           identified{false}; // Writing gates
    std::vector<double>         E;   // Energy considering energy losses
    std::vector<double>         ESi; // Energy detected from calibration
    std::vector<double>         Ex;  // Excitation energy
    std::vector<TVector3>       Pos; // Hit position
    std::vector<TLorentzVector> p4;  // 4 momentum of recoil
    std::vector<TLorentzVector> p3;  // 4 momentum of recoil
  bool sauronGood = false;
  void                        resize(const unsigned int& mult) override {
        Container::resize(mult);
        IdPside.resize(mult);
        IdNside.resize(mult);
        A.resize(mult);
        Z.resize(mult);
        identified.resize(mult);
        E.resize(mult);
        ESi.resize(mult);
        Ex.resize(mult);
        Pos.resize(mult);
        p4.resize(mult);
        p3.resize(mult);
    }
    void clear() override {
        Container::clear();
        IdPside.clear();
        IdNside.clear();
        A.clear();
        Z.clear();
        identified.clear();
        E.clear();
        ESi.clear();
        Ex.clear();
        Pos.clear();
        p3.clear();
        p4.clear();
    }
    Sauron() = default;
    ClassDefOverride(Sauron, 2);
};
