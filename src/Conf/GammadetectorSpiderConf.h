#pragma once

#include "CoincidenceConf.h"
#include "GammadetectorSpiderLut.h"
#include <cmath>
#include <string>
#include <vector>

class GammadetectorSpiderConf : public CoincidenceConf {
  public:
    GammadetectorSpiderConf() = delete;

  protected:
    explicit GammadetectorSpiderConf(const std::string& mark);

  public:
    GammadetectorSpiderLut   lut;
    std::vector<std::string> lutFiles{
        "none",
    };
    std::vector<std::pair<double, double>> exValues{
        {0, 1}
    };

    struct GammaGate {
        std::pair<int, int> gate{0, 1};
        std::string         option{"None"};
        std::pair<int, int> exclude_angle{80, 100};
    };

    std::vector<GammaGate> gammaGates;
    std::vector<GammaGate> exgammaGates;


  inline bool
    isDeclared(const unsigned int& gatenum, const unsigned int& id) {
        return lut.isDeclared(gatenum, floor(id / 16), id % 16);
    };

    inline bool isInside(const unsigned int& gatenum, const unsigned int& id,
                         const double& Ex) {
        return lut.isInside(gatenum, floor(id / 16), id % 16, Ex);
    };

    std::ostream&          operator<<(std::ostream& os) const override;
    std::istream&          operator>>(std::istream& is) override;

  protected:
    std::unordered_map<std::string, Property<std::vector<GammaGate>&>> parGG;
};

std::istream&
operator>>(std::istream& is, GammadetectorSpiderConf::GammaGate& vec);
std::ostream&
operator<<(std::ostream& os, const GammadetectorSpiderConf::GammaGate& vec);
GammadetectorSpiderConf::GammaGate
operator*=(GammadetectorSpiderConf::GammaGate& v, const double& d);
