#pragma once

#include <TLorentzVector.h>
#include <TVector3.h>

#include "Container.h"

class Gammadetector : public Container {
  protected:
    Gammadetector() = default;

  public:
    std::vector<double>         E;      //
    std::vector<double>         EDC;    // with betaAvg
    std::vector<TVector3>       Pos;    //
    std::vector<int>            CoreID; //
    std::vector<TLorentzVector> P;      //
    void                        resize(const unsigned int& mult) override {
        Container::resize(mult);
        E.resize(mult);
        EDC.resize(mult);
        Pos.resize(mult);
        CoreID.resize(mult);
        P.resize(mult);
    };

    void clear() override {
        Container::clear();
        E.clear();
        EDC.clear();
        Pos.clear();
        CoreID.clear();
        P.clear();
    }
    ClassDefOverride(Gammadetector, 2);
};
