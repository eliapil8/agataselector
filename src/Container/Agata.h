#pragma once

#include <TLorentzVector.h>
#include <TVector3.h>

#include "Gammadetector.h"

class Agata : public Gammadetector {
  public:
    void resize(const unsigned int& mult) override {
        Gammadetector::resize(mult);
    };
    void clear() override { Gammadetector::clear(); }
    ClassDefOverride(Agata, 2);
};
