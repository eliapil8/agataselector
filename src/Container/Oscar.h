#pragma once

#include <TLorentzVector.h>

#include <iostream>
#include <string>

#include "Container.h"

class Oscar : public Container
{
  public:
    std::vector<unsigned short> IdTelescope;       // ID
    std::vector<unsigned short> IddE;              // ID dE layer
    std::vector<unsigned short> IdE;               // ID E layer
    std::vector<unsigned short> A;                 // ID A
    std::vector<unsigned short> Z;                 // ID Z
    std::vector<bool>           identified{false}; // Writing gates
    std::vector<double>         ESi;               // Energy detected from calibration
    std::vector<double>         dESi;              // Energy first layer detected from calibration
    std::vector<double>         E;                 // Energy considering energy losses
    std::vector<double>         Ex;                // Excitation energy
    std::vector<TVector3>       Pos;               // Hit position
    std::vector<TLorentzVector> p4;                // 4 momentum of recoil
    std::vector<TLorentzVector> p3;                // 4 momentum of recoil
    void                        resize(const unsigned int &mult) override
    {
        Container::resize(mult);
        IdTelescope.resize(mult);
        IddE.resize(mult);
        IdE.resize(mult);
        A.resize(mult);
        Z.resize(mult);
        identified.resize(mult);
        E.resize(mult);
        dESi.resize(mult);
        ESi.resize(mult);
        Ex.resize(mult);
        Pos.resize(mult);
        p4.resize(mult);
        p3.resize(mult);
    }
    void clear() override
    {
        Container::clear();
        IddE.clear();
        IdE.clear();
        A.clear();
        Z.clear();
        identified.clear();
        E.clear();
        dESi.clear();
        ESi.clear();
        Ex.clear();
        Pos.clear();
        p4.clear();
        p3.clear();
    }
    Oscar() = default;
    ClassDefOverride(Oscar, 4);
};
