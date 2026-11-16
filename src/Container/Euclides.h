#pragma once

#include <TLorentzVector.h>
#include <TVector3.h>

#include <iostream>
#include <string>

#include "Container.h"

class Euclides : public Container {
  public:
    std::vector<unsigned int> Id; // ID
    std::vector<double>       E;  // Energy with Energy loss corrections
    std::vector<double>
        dESi; // Calibrated Energy with no Energy loss corrections for dE layer
    std::vector<double> ESi; // Calibrated Energy of E+dE energy with no energy
                             // loss corrections
    double              Ex{
        0}; // Excitation energy of the compond system based on the kinematics
    std::vector<unsigned int>   A;                 // ID A
    std::vector<unsigned int>   Z;                 // ID Z
    std::vector<bool>           identified{false}; // Witing gates
    TLorentzVector              p3;                // 4 momentum of recoil
    std::vector<TLorentzVector> pFragments; // 4 momentum of detected particles
    std::vector<TLorentzVector>
        gammaDCBP; // gamma correction based on kinematic calculations
    std::vector<TVector3> pos; // Position of detection
    void                  resize(const unsigned int& mult) override {
        Container::resize(mult);
        Id.resize(mult);
        E.resize(mult);
        dESi.resize(mult);
        ESi.resize(mult);
        A.resize(mult);
        Z.resize(mult);
        identified.resize(mult);
        pFragments.resize(mult);
        gammaDCBP.resize(mult);
        pos.resize(mult);
    }

    void clear() override {
        Container::clear();
        Id.clear();
        E.clear();
        dESi.clear();
        ESi.clear();
        Ex = 0;
        A.clear();
        Z.clear();
        identified.clear();
        p3 = TLorentzVector();
        pFragments.clear();
        gammaDCBP.clear();
        pos.clear();
    }
    Euclides() = default;
    ClassDefOverride(Euclides, 6);
};
