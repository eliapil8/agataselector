#pragma once

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Conf.h"
#include "Fitter.h"
#include "Units.h"

class OptimizerConf : public Conf {
  public:
    OptimizerConf();

    void          RetrieveParameters();

    std::ostream& operator<<(std::ostream& os) const override;
    std::istream& operator>>(std::istream& is) override;

    struct Parameter {
        std::string detector;
        std::string name;
        double      initialValue;
        double      minValue;
        double      maxValue;
        double      step;
        double*     ptr;
    };
    struct ContourParameter {
        std::string  detector1;
        std::string  name1;
        std::string  detector2;
        std::string  name2;
        unsigned int nPoints;
    };

    std::vector<Parameter>        parameters{};
    std::vector<Parameter>        parameterScan{};
    std::vector<Parameter>        transition{};
    std::vector<ContourParameter> parameterContour{};

    // std::string
    // spectrum{"Agata/Ana/h_EDC_avg"};
    std::string                   fitParFile{"parameters.dat"};
    std::string                   logFile{"log.txt"};
    std::string                   optimizerOutputFile{"out.root"};
    std::string                   minimizerName{"Minuit"};
    std::string                   minimizerAlgorithm{"Simplex"};
    int                           maxCalls{1000};
    int                           printLevel{1};
    int                           bkgPolOrder{1};
    double                        tolerance{0.1};
    double                        precision{0.01};
    double                        sigmaWeight{0.5};
    double                        nonConvergenceCost{5};
    int                           tail{Fitter::Tail::NOTAIL};
    bool                          useIntervals{false};
    bool                          validErrors{false};
    bool                          saveScanFiles{false};
    bool                          onlyScan{false};
    // bool
    // debugCanvas{false}; std::vector<std::pair<double, double>> transitions{
    //	{1400 * UNITS::keV, 100}
    // };

    void setOptimizablePtr(const std::string& det, const std::string& par,
                           double* const& ptr);

  protected:
    std::unordered_map<std::string, Property<std::vector<Parameter>&>> parVPARS;
    std::unordered_map<std::string, Property<std::vector<ContourParameter>&>>
        parVPARSC;

  private:
    std::unordered_map<std::string,
                       std::unordered_map<std::string, double* const>>
        nameToPtr;
};

std::istream& operator>>(std::istream& is, OptimizerConf::Parameter& v);
std::ostream& operator<<(std::ostream& os, const OptimizerConf::Parameter& v);
OptimizerConf::Parameter
              operator*=(OptimizerConf::Parameter& v, const double& d);

std::istream& operator>>(std::istream& is, OptimizerConf::ContourParameter& v);
std::ostream&
operator<<(std::ostream& os, const OptimizerConf::ContourParameter& v);
OptimizerConf::ContourParameter
operator*=(OptimizerConf::ContourParameter& v, const double& d);
