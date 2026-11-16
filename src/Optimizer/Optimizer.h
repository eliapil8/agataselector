#pragma once

#include "ConfManager.h"
#include "Fitter.h"
#include "Settings.h"
#include "Units.h"

#include <TMinuit.h>

class Optimizer {
  public:
    Optimizer();
    ~Optimizer() { delete log; }

  private:
    const Settings&   settings;
    ConfManager&      conf;

    // TMinuit minimizer;
    void              Child();
    void              Parent();

    double            Cost(const double* pars);
    std::vector<TH1D> retrieveSpectrum(const std::string&);

    int               receiveFd{};
    FILE*             receiveF{};
    int               sendFd{};
    pid_t             pid;

    std::ofstream*    log{nullptr};
    int               iteration{0};

    struct Data {
        bool         terminateChild{false};
        bool         printConf{false};
        bool         isScan{false};
        unsigned int scanParNr{0};
        char         parameterName[500];
        double       cost{};
        double       mean{};
        double       sigma{};
        double       pars[500]{};
    };
    Data newData;

    Data receiveData() const;
    void sendData(const Data&) const;
};
