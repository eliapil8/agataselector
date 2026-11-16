#pragma once

#include "DetectorConf.h"

class GammadetectorConf : public DetectorConf {
  public:
    GammadetectorConf() = delete;

  protected:
    explicit GammadetectorConf(const std::string& mark);

  public:
    int    binWidth{1};
    double gammaEmax{4000 * UNITS::keV};
    bool   excludeAncillary{false};
    enum SpectraType {
        SEGMENT,
        CORE,
        CORE1,
        ADDBACK,
        TRACKED,
        CALORIMETER
    } spectraType
        = CORE;

    struct GammaGate {
        std::pair<int, int> gate{0, 1};
        std::string         option{"None"};
        std::pair<int, int> exclude_angle{80, 100};
    };

    std::vector<GammaGate>                 gammaGates;
    std::vector<std::pair<double, double>> excludeTimestamp{
        {0, 0}
    };

    std::ostream& operator<<(std::ostream& os) const override;
    std::istream& operator>>(std::istream& is) override;

  protected:
    std::unordered_map<std::string, Property<std::vector<GammaGate>&>> parGG;
    std::unordered_map<std::string, Property<SpectraType&>>            parST;

  private:
    static SpectraType ClassifySpectra(const std::string& val);
    static std::string SpectraName(const SpectraType& s);
};

std::istream& operator>>(std::istream& is, GammadetectorConf::GammaGate& vec);
std::ostream&
operator<<(std::ostream& os, const GammadetectorConf::GammaGate& vec);
GammadetectorConf::GammaGate
operator*=(GammadetectorConf::GammaGate& v, const double& d);
