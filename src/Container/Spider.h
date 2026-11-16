#pragma once

#include <TLorentzVector.h>

#include <iostream>
#include <string>

#include "Container.h"

class Spider : public Container {
  public:
    std::vector<unsigned int>   Id;  // ID
    std::vector<double>         E;   // Energy considering energy losses
    std::vector<double>         ESi; // Energy detected from calibration
    std::vector<double>         Ex;  // Excitation energy
    std::vector<TLorentzVector> p4;  // 4 momentum of recoil
    std::vector<TLorentzVector> p3;  // 4 momentum of recoil
    void                        resize(const unsigned int& mult) override {
        Container::resize(mult);
        Id.resize(mult);
        E.resize(mult);
        ESi.resize(mult);
        Ex.resize(mult);
        p4.resize(mult);
        p3.resize(mult);
    }
    void clear() override {
        Container::clear();
        Id.clear();
        E.clear();
        ESi.clear();
        Ex.clear();
        p3.clear();
        p4.clear();
    }
    Spider() = default;
    ClassDefOverride(Spider, 4);
};
