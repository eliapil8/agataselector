#pragma once

#include "TLorentzVector.h"

#include <iostream>
#include <string>

#include "Container.h"

class SauronMcp : public Container {
public:
  std::vector<double>         Ex;  // Excitation energy
  std::vector<TLorentzVector> p3;  // 4 momentum of BP
  std::vector<TLorentzVector> p4;  // 4 momentum of BP
  bool sauronMcpGood = false;
  bool sauronMcpAgataGood = false;
  void                        resize(const unsigned int& mult) override {
    Container::resize(mult);
    Ex.resize(mult);
    p4.resize(mult);
    p3.resize(mult);
  }
  void clear() override {
    Container::clear();
    Ex.clear();
    p4.clear();
    p3.clear();
    sauronMcpGood = false;
    sauronMcpAgataGood = false;
  }
  SauronMcp() = default;
  ClassDefOverride(SauronMcp, 2);
};
