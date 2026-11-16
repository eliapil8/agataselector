#pragma once

#include <TLorentzVector.h>

#include <iostream>
#include <string>

#include "Gammadetector.h"

class Labr : public Gammadetector {
  public:
    std::vector<unsigned int> Id;
    std::vector<double>       Qshort;
    double       MonE;
    double       MonTime;
    void         resize(const unsigned int& mult) override {
        Gammadetector::resize(mult);
        Id.resize(mult);
        Qshort.resize(mult);
    }
    void clear() override {
        Gammadetector::clear();
        Id.clear();
        Qshort.clear();
        MonE = 0;
        MonTime = 0;
    }
    Labr() = default;
    ClassDefOverride(Labr, 4);
};
