#pragma once

#include <TCanvas.h>
#include <TF1.h>
#include <TFitResult.h>
#include <TH1.h>

#include <RooAddPdf.h>
#include <RooConstVar.h>
#include <RooDataHist.h>
#include <RooDataSet.h>
#include <RooDecay.h>
#include <RooFitResult.h>
#include <RooGExpModel.h>
#include <RooGaussModel.h>
#include <RooHist.h>
#include <RooPlot.h>
#include <RooPolynomial.h>
#include <RooRealVar.h>

#include <fstream>
#include <memory>
#include <sstream>
#include <unordered_map>

class Fitter : public TObject {
  public:
    enum Tail { NOTAIL, RIGHT, LEFT, LEFT_RIGHT, SYMMETRIC };
    explicit Fitter(std::vector<std::pair<double, double>>,
                    const Tail& = NOTAIL, const int& pLevel=0);
    Fitter() = delete;
    ~Fitter() override;

    // Structure of fit results
    struct FitRes {
        int                       status{-1};
        double                    energy;
        std::pair<double, double> integral;
        std::pair<double, double> counts;
        std::pair<double, double> mean_gauss;
        std::pair<double, double> sigma_gauss;
        std::pair<double, double> tail_left;
        std::pair<double, double> tail_right;
        FitRes(const double& energy, const double& integral,
               const double& integral_err, const double& counts,
               const double& counts_err, const double& mean_gauss,
               const double& mean_gauss_err, const double& sigma_gauss,
               const double& sigma_gauss_err, const double& tail_left,
               const double& tail_left_err, const double& tail_right,
               const double& tail_right_err)
            : energy(energy), integral(std::make_pair(integral, integral_err)),
              counts(std::make_pair(counts, counts_err)),
              mean_gauss(std::make_pair(mean_gauss, mean_gauss_err)),
              sigma_gauss(std::make_pair(sigma_gauss, sigma_gauss_err)),
              tail_left(std::make_pair(tail_left, tail_left_err)),
              tail_right(std::make_pair(tail_right, tail_right_err)) {}
        // ClassDef(FitRes, 1);
    };

    // Interface methods
    void EnableCanvas(bool cv_enabled = true);
    void SetMeanIntervalAroundCenter(const double& min, const double& max);
    void SetFitLimits(const double& min, const double& max);
    void SetMeanFixed(const bool& mean_fix = true);
    void SetRelativeAmplitudeFixed(const bool& relative_amplitude_fix = true);
    void WriteParsOnFile(const std::string&, const unsigned int&);
    void ReadParsFromFile(const std::string&, const unsigned int&);
    void SetEnergyUnits(const double&);
    void SetBkgOrder(const int&);
    void UseMinos(const bool& use_minos = true);
    void SetSigmaLimits(const std::pair<double, double>&, const double&); // added units
    void SetTauLimits(const std::pair<double, double>&);
    void SetInitialSigma(const double&, const double&); // added units 
    void SetInitialTau(const double&);
    void
    SaveParsOnFile(const std::vector<Fitter::FitRes>& par, char* fileName);

    std::vector<FitRes> Fit();
    TH1D&               GetSpecRef();
    TF1&                GetFitRef();
    void                SetSpec(const TH1D& s);

  private:
    void   FindParameters();
    double mean_around_center_min{5.}, mean_around_center_max{5.};
    TF1    fitfunc;
    TH1D   spec;
    Tail   settings_tail{NOTAIL};
    std::vector<std::pair<double, double>> energies;
    int    printLevel{5};
    bool                                   canvas_enabled{false};
    bool                                   mean_fixed{false};
    bool                                   relative_amplitude_fixed{false};
    bool                                   minos{false};
    unsigned int                           bkg_pol_order{2};
    std::pair<double, double>              fit_limits{7, 7};

  // Single parameter values
  public:
    struct InitialState {
        double                    initial_value{0};
        std::pair<double, double> fitted_value{0, 0};
        double                    min{0};
        double                    max{0};
        bool                      fixed{false};
        std::string               name{};
        // InitialState(double initial_value, double min, double max, bool
        // fixed)
        //     : initial_value(initial_value), min(min), max(max), fixed(fixed)
        //     {}
        // InitialState() : initial_value(0), min(0), max(0), fixed(false) {}
        void                      GetParameter(const RooRealVar& par) {
            fitted_value.first  = par.getVal();
            fitted_value.second = par.getError();
        }
        // ClassDef(InitialState, 1);
    };

    struct RooFitParams {
        RooRealVar* fracsignal;
        RooRealVar* mean;
        RooRealVar* sigma;
        RooRealVar* tau_left;
        RooRealVar* tau_right;
        RooFitParams(RooRealVar* fracsignal, RooRealVar* mean,
                     RooRealVar* sigma, RooRealVar* tau_left,
                     RooRealVar* tau_right)
            : fracsignal(fracsignal), mean(mean), sigma(sigma),
              tau_left(tau_left), tau_right(tau_right){};
        ~RooFitParams() = default; // Otherwise roofit crashes
        //~RooFitParams(){
        //    delete fracsignal;
        //    delete mean;
        //    delete sigma;
        //    delete tau_left;
        //    delete tau_right;
        //}
        // ClassDef(RooFitParams, 1);
    };

    // All parameters together
    struct InitializationParameters {
        std::vector<InitialState> ampl;
        std::vector<InitialState> mean;
        std::vector<InitialState> sigma;
        std::vector<InitialState> tau_left;
        std::vector<InitialState> tau_right;

        double                    left_interval{};
        double                    right_interval{};
        double                    initial_sigma{1.3};
        double                    min_sigma{0.5};
        double                    max_sigma{10.};
        double                    initial_tau{0.1};
        double                    min_tau{0.001};
        double                    max_tau{10};

        explicit InitializationParameters(unsigned int n_gauss) {
            Resize(n_gauss);
        };
        void Resize(unsigned int& n_gauss) {
            ampl.resize(n_gauss);
            mean.resize(n_gauss);
            sigma.resize(n_gauss);
            tau_left.resize(n_gauss);
            tau_right.resize(n_gauss);
        };

        explicit InitializationParameters() = default;
        // InitializationParameters(const InitializationParameters&
        // old_obj)=default;
        void GetParameters(std::vector<RooFitParams>& params) {
            for(unsigned long int ii = 0; ii < ampl.size(); ++ii) {
                ampl[ii].GetParameter(*params[ii].fracsignal);
                mean[ii].GetParameter(*params[ii].mean);
                sigma[ii].GetParameter(*params[ii].sigma);
                tau_left[ii].GetParameter(*params[ii].tau_left);
                tau_right[ii].GetParameter(*params[ii].tau_right);
            }
        }
        // ClassDef(InitializationParameters, 1);
    };

  private:
    InitializationParameters  parameters;
    std::vector<InitialState> bkgInitialParameters;

    // ClassDefOverride(Fitter, 3);
};
