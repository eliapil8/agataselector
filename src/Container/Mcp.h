#pragma once

#include <iostream>
#include <vector>
#include <array>
#include <TLorentzVector.h>
#include <TVector3.h>

#include "Container.h"

class Mcp : public Container {
  public:
    std::array<std::array<double, 8>, 2> TOF; // TOF
    std::array<std::array<double, 8>, 2> TOFcal; // TOF
    TLorentzVector p1;  // 4 momentum of beam
    std::array<TVector3,2>      pos{TVector3(9999,9999,9999),TVector3(9999,9999,9999)}; // Position of detection on MCPs
    bool                        bothMCP{false}; // Both MCPs are present
    bool                        good{false}; // Both MCPs are present and the event reach the (0,0,0)
    bool                        histopos{false};
    bool                        bline1{false};
    bool                        bline2{false};
    TVector3       postarget{9999,9999,9999}; // Position of detection on target

    void                        resize(const unsigned int& mult) override {
        Container::resize(mult);
    }

    void clear() override {
        Container::clear();
        TOF = {};
        TOFcal = {};
        pos={TVector3(9999,9999,9999),TVector3(9999,9999,9999)};
        postarget={9999,9999,9999};
        p1={};
        bothMCP = false;
        good = false;
        histopos = false;
        bline1 = false;
        bline2 = false;
    }
    Mcp() = default;
    ClassDefOverride(Mcp, 2);
};