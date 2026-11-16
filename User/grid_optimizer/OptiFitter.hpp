#pragma once

#include "TF1.h"
#include "TFitResult.h"
#include "TH1.h"

#include <cmath>
#include <mutex>

#include <limits>

namespace Detail
{
double constexpr sqrtNewtonRaphson(double x, double curr, double prev)
{
    return curr == prev ? curr : sqrtNewtonRaphson(x, 0.5 * (curr + x / curr), curr);
}
} // namespace Detail

/*
 * Constexpr version of the square root
 * Return value:
 *	- For a finite and non-negative value of "x", returns an approximation for the square root of "x"
 *   - Otherwise, returns NaN
 */
double constexpr ce_sqrt(double x)
{
    return x >= 0 && x < std::numeric_limits<double>::infinity() ? Detail::sqrtNewtonRaphson(x, x, 0)
                                                                 : std::numeric_limits<double>::quiet_NaN();
}

namespace gridopti
{

constexpr double PI              = 3.14159265358979323846;
constexpr double SQRT_2          = ce_sqrt(2);
constexpr double one_over_SQRT_2 = 1 / SQRT_2;
constexpr double SQRT_2PI        = ce_sqrt(2 * PI);

enum optiFitterType
{
    kVoid_type      = 0,
    kLorenz         = 1,
    kSkewedGaussian = 2
};

enum optiFitStrategy
{
    kVoid_strategy     = 0,
    kDynamic_FWHM      = 1,
    kStatic_WidthAtVal = 2
};

struct optiFitterParameters_t
{
    optiFitterType  type{kSkewedGaussian};
    optiFitStrategy strategy{kDynamic_FWHM};
    double          strategy_value{5};
} gOptiFitterParameters;

struct optiPeakFitSettings_t
{
    std::vector<double> enMin;
    std::vector<double> enMax;
    std::vector<double> enPeak;

    void AddPeak(double e_min, double e_max, double e_peak)
    {
        enMin.push_back(e_min);
        enMax.push_back(e_max);
        enPeak.push_back(e_peak);
    }

    void Clear()
    {
        enMin.clear();
        enMax.clear();
        enPeak.clear();
    }
} gOptiPeakFitSettings;

class OptiFitter
{
  public:
    OptiFitter() : fFitResults({0, 0, 0, 0, 0}), fEnMin(0), fEnMax(0), fRangeSet(false){};

    OptiFitter(double emin, double emax) : fFitResults({0, 0, 0, 0, 0}), fEnMin(emin), fEnMax(emax), fRangeSet(true)
    {
        assert(emin < emax && "emin must be smaller than emax");
    };
    ~OptiFitter(){};
    virtual bool FitPeak(TH1 &histo, double emin, double emax) = 0;
    virtual bool FitPeak(TH1 &histo)                           = 0;

    /// @brief Get results of the latest fit,
    /// @return 0 = volume, 1 = mean, 2 = width, 3 and 4 optional (3 = skewness for skewed gaussian)
    const std::array<double, 5>& GetFitResults() const noexcept { return fFitResults; }

    /// @brief set energy range for fitting
    /// @param xmin
    /// @param xmax
    void SetRange(double xmin, double xmax) noexcept
    {
        fEnMin    = xmin;
        fEnMax    = xmax;
        fRangeSet = true;
    }
    /// @brief Estimate the FWHM, FWFM, FWTM etc of a given function, this already asumes the background parameters are zeroed
    /// @param func
    /// @param xMin
    /// @param xMax
    /// @param width_fraction
    /// @return
    inline double EstimateFWxM(const Double_t &xMin, const Double_t &xMax, const double &width_multiple,
                               const unsigned int mean_par_index = 1);

    /// @brief similar as EstimateFWxM, but estimates the width of a function at a given Y threshold value instead of a fraction of the
    /// maximum
    /// @param func
    /// @param xMin
    /// @param xMax
    /// @param static_y_threshold
    /// @return
    inline double EstimateWidth(const Double_t &xMin, const Double_t &xMax, const double &static_y_threshold,
                                const unsigned int mean_par_index = 1);

  protected:
    /// @brief result array, 0 = volume, 1 = mean, 2 = width, 3 and 4 optional (3 = skewness for skewed gaussian)
    std::array<double, 5> fFitResults;
    /// @brief min range for fitting
    double fEnMin;
    /// @brief max range for fitting
    double fEnMax;
    /// @brief flag to check if the range is set
    bool fRangeSet;
    /// @brief fit function
    std::unique_ptr<TF1> fPeakFcn;
    /// @brief mutex for the fitting
    static std::mutex fFitLock;
};

std::mutex gridopti::OptiFitter::fFitLock;

double OptiFitter::EstimateFWxM(const Double_t &xMin, const Double_t &xMax, const double &width_multiple, const unsigned int mean_par_index)
{
    const double fraction_max = fPeakFcn->GetMaximum() / width_multiple;
    return this->EstimateWidth(xMin, xMax, fraction_max, mean_par_index);
}

double OptiFitter::EstimateWidth(const Double_t &xMin, const Double_t &xMax, const double &static_y_threshold,
                                 const unsigned int mean_par_index)
{
    double left  = fPeakFcn->GetX(static_y_threshold, xMin, fPeakFcn->GetParameter(mean_par_index), 1E-1, 100, false);
    double right = fPeakFcn->GetX(static_y_threshold, fPeakFcn->GetParameter(mean_par_index), xMax, 1E-1, 100, false);
    return right - left;
}
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

double lorenz_with_background_fcn(const double *x, const double *p)
{
    return p[0] / PI * p[2] / 2. / (TMath::Power(x[0] - p[1], 2.) + p[2] * p[2] / 4.) + p[3] + p[4] * x[0];
}

class OptiFitterLorenz : public OptiFitter
{

  public:
    OptiFitterLorenz() : OptiFitter()
    {
        fPeakFcn = std::make_unique<TF1>("lorenz_with_background_fcn", lorenz_with_background_fcn, fEnMin, fEnMax, 5);
        fPeakFcn->SetParNames("volume", "mean", "width", "bcg_0", "bcg_1");
        fPeakFcn->SetLineColor(kGreen);
    }

    OptiFitterLorenz(double emin, double emax) : OptiFitter(emin, emax)
    {
        fPeakFcn = std::make_unique<TF1>("lorenz_with_background_fcn", lorenz_with_background_fcn, emin, emax, 5);
        fPeakFcn->SetParNames("volume", "mean", "width", "bcg_0", "bcg_1");
        fPeakFcn->SetLineColor(kGreen);
    }

    bool FitPeak(TH1 &histo, double emin, double emax) override
    {
        assert(emin < emax && "emin must be smaller than emax");

        histo.GetXaxis()->SetRangeUser(emin, emax);

        double xmin    = emin;
        double xmax    = emax;
        int    bin_min = histo.FindBin(xmin);
        int    bin_max = histo.FindBin(xmax);
        fPeakFcn->SetNpx((bin_max - bin_min) * 100);

        double y_min = histo.GetBinContent(bin_min);
        double y_max = histo.GetBinContent(bin_max);

        // estimate background
        double p1 = (y_max - y_min) / (xmax - xmin);
        double p0 = y_min - p1 * xmin;
        fPeakFcn->SetParameter(3, p0);
        fPeakFcn->SetParameter(4, p1);

        // estimate mean
        int bin_max_val = histo.GetMaximumBin();
        fPeakFcn->SetParameter(1, histo.GetBinCenter(bin_max_val));
        fPeakFcn->SetParLimits(1, xmin, xmax);

        // estimate volume
        const double bcg_counts  = 0.5 * (xmax - xmin) * (p1 * (xmin + xmax) + 2. * p0);
        const double all_counts  = histo.Integral(bin_min, bin_max);
        const double peak_volume = all_counts - bcg_counts;
        assert(peak_volume > 0 && "estimated volume of Lorenzian appears to be negative");
        fPeakFcn->SetParameter(0, peak_volume);
        fPeakFcn->SetParLimits(0, peak_volume / 100., all_counts * 2.); // reasonable limits to prevent ROOT going crazy

        // estimate width (same as FWHM or Gamma)
        double amplitude = histo.GetBinContent(bin_max_val);
        double width     = 2. / TMath::Pi() * peak_volume / amplitude;
        fPeakFcn->SetParameter(2, width);
        fPeakFcn->SetParLimits(2, 0, width * 10); // "reasonable" limits to prevent ROOT going crazy

        // fit in range, quiet mode, do not draw, use integral, improve result
        std::lock_guard<std::mutex> guard(fFitLock);
        TFitResultPtr               fitptr = histo.Fit(fPeakFcn.get(), "SRQMI0");
        // so that function will be drawn together with histo in the future
        histo.GetFunction(fPeakFcn->GetName())->ResetBit(TF1::kNotDraw);

        fFitResults[0] = fPeakFcn->GetParameter(0);
        fFitResults[1] = fPeakFcn->GetParameter(1);
        fFitResults[2] = fPeakFcn->GetParameter(2);
        return fitptr->IsValid();

        // calculate peak volume, set bcg to zero
        p0 = fPeakFcn->GetParameter(3);
        p1 = fPeakFcn->GetParameter(4);
        fPeakFcn->SetParameter(3, 0);
        fPeakFcn->SetParameter(4, 0);

        double final_width;
        if (gOptiFitterParameters.strategy == kDynamic_FWHM)
            final_width = EstimateFWxM(xmin, xmax, gOptiFitterParameters.strategy_value, 1);
        else
            final_width = EstimateWidth(xmin, xmax, gOptiFitterParameters.strategy_value, 1);

        fFitResults[0] = fPeakFcn->GetParameter(0);
        fFitResults[1] = fPeakFcn->GetParameter(1);
        fFitResults[2] = final_width;

        // just in case
        fPeakFcn->SetParameter(3, p0);
        fPeakFcn->SetParameter(4, p1);

        return fitptr->IsValid();
    }

    bool FitPeak(TH1 &histo) override
    {
        double xmin, xmax;
        int    bin_min, bin_max;
        if (!fRangeSet)
        {
            xmin = histo.GetXaxis()->GetXmin();
            xmax = histo.GetXaxis()->GetXmax();

            bin_min = histo.FindBin(xmin);
            bin_max = histo.FindBin(xmax);

            fPeakFcn->SetRange(xmin, xmax);
            fPeakFcn->SetNpx(histo.GetNbinsX() * 100);
        }
        else
        {
            xmin = fEnMin;
            xmax = fEnMax;

            bin_min = histo.FindBin(xmin);
            bin_max = histo.FindBin(xmax);

            fPeakFcn->SetRange(xmin, xmax);
            fPeakFcn->SetNpx((bin_max - bin_min) * 100);
        }

        double y_min = histo.GetBinContent(bin_min);
        double y_max = histo.GetBinContent(bin_max);

        // estimate background
        double p1 = (y_max - y_min) / (xmax - xmin);
        double p0 = y_min - p1 * xmin;
        fPeakFcn->SetParameter(3, p0);
        fPeakFcn->SetParameter(4, p1);

        // estimate mean
        int bin_max_val = histo.GetMaximumBin();
        fPeakFcn->SetParameter(1, histo.GetBinCenter(bin_max_val));
        fPeakFcn->SetParLimits(1, xmin, xmax);

        // estimate volume
        const double bcg_counts  = 0.5 * (xmax - xmin) * (p1 * (xmin + xmax) + 2. * p0);
        const double all_counts  = histo.Integral(bin_min, bin_max);
        const double peak_volume = all_counts - bcg_counts;
        assert(peak_volume > 0 && "estimated volume of Lorenzian appears to be negative");
        fPeakFcn->SetParameter(0, peak_volume);
        fPeakFcn->SetParLimits(0, peak_volume / 100., all_counts * 2.); // reasonable limits to prevent ROOT going crazy

        // estimate width (same as FWHM or Gamma)
        double amplitude = histo.GetBinContent(bin_max_val);
        double width     = 2. / TMath::Pi() * peak_volume / amplitude;
        fPeakFcn->SetParameter(2, width);
        fPeakFcn->SetParLimits(2, 0, width * 10); // "reasonable" limits to prevent ROOT going crazy

        // fit in range, quiet mode, do not draw, use integral, improve result
        std::lock_guard<std::mutex> guard(fFitLock);
        TFitResultPtr               fitptr = histo.Fit(fPeakFcn.get(), "SRQMI0");
        // so that function will be drawn together with histo in the future
        histo.GetFunction(fPeakFcn->GetName())->ResetBit(TF1::kNotDraw);

        // calculate peak volume, set bcg to zero
        p0 = fPeakFcn->GetParameter(3);
        p1 = fPeakFcn->GetParameter(4);
        fPeakFcn->SetParameter(3, 0);
        fPeakFcn->SetParameter(4, 0);

        double final_width;
        if (gOptiFitterParameters.strategy == kDynamic_FWHM)
            final_width = EstimateFWxM(xmin, xmax, gOptiFitterParameters.strategy_value, 1);
        else
            final_width = EstimateWidth(xmin, xmax, gOptiFitterParameters.strategy_value, 1);

        fFitResults[0] = fPeakFcn->GetParameter(0);
        fFitResults[1] = fPeakFcn->GetParameter(1);
        fFitResults[2] = final_width;

        // just in case
        fPeakFcn->SetParameter(3, p0);
        fPeakFcn->SetParameter(4, p1);

        return fitptr->IsValid();
    }
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

Double_t skewed_gaussian_with_background(Double_t *x, Double_t *par)
{
    // Parameters: par[0] = norm, par[1] = mean, par[2] = sigma, par[3] = alpha, par[4] = bcg_0, par[5] = bcg_1
    Double_t arg      = (x[0] - par[1]) / par[2];
    Double_t erf_term = TMath::Erf(par[3] * arg * one_over_SQRT_2);
    return par[0] / (par[2] * SQRT_2PI) * TMath::Exp(-0.5 * arg * arg) * (1 + erf_term) + par[4] + par[5] * x[0];
}

class OptiFitterSkewedGaussian : public OptiFitter
{

  public:
    OptiFitterSkewedGaussian() : OptiFitter()
    {
        fPeakFcn = std::make_unique<TF1>("skewed_gaussian_with_background_fcn", skewed_gaussian_with_background, fEnMin, fEnMax, 6);
        fPeakFcn->SetParNames("norm", "mean", "sigma", "alpha", "bcg_0", "bcg_1");
    }

    OptiFitterSkewedGaussian(double emin, double emax) : OptiFitter(emin, emax)
    {
        fPeakFcn = std::make_unique<TF1>("skewed_gaussian_with_background_fcn", skewed_gaussian_with_background, emin, emax, 6);
        fPeakFcn->SetParNames("norm", "mean", "sigma", "alpha", "bcg_0", "bcg_1");
    }

    bool FitPeak(TH1 &histo, double emin, double emax) override
    {
        assert(emin < emax && "emin must be smaller than emax");

        histo.GetXaxis()->SetRangeUser(emin, emax);


        double xmin    = emin;
        double xmax    = emax;
        int    bin_min = histo.FindBin(xmin);
        int    bin_max = histo.FindBin(xmax);
        fPeakFcn->SetNpx((bin_max - bin_min) * 10);

        double y_min = histo.GetBinContent(bin_min);
        double y_max = histo.GetBinContent(bin_max);

        // estimate background
        double p1 = (y_max - y_min) / (xmax - xmin);
        double p0 = y_min - p1 * xmin;
        fPeakFcn->SetParameter(4, p0);
        fPeakFcn->SetParameter(5, p1);

        // set skewness to zero - assume regular gaussian first
        fPeakFcn->SetParameter(3, 0.);
        fPeakFcn->SetParLimits(3, 0., 100.);

        // estimate mean
        int bin_max_val = histo.GetMaximumBin();
        fPeakFcn->SetParameter(1, histo.GetBinCenter(bin_max_val));
        fPeakFcn->SetParLimits(1, xmin, xmax);

        // estimate volume - assuming regular Gaussian
        const double bcg_counts  = 0.5 * (xmax - xmin) * (p1 * (xmin + xmax) + 2. * p0);
        const double all_counts  = histo.Integral(bin_min, bin_max);
        const double peak_volume = all_counts - bcg_counts;
        assert(peak_volume > 0 && "estimated volume of Lorenzian appears to be negative");
        fPeakFcn->SetParameter(0, peak_volume);
        fPeakFcn->SetParLimits(0, all_counts / 100., all_counts * 100.); // reasonable limits to prevent ROOT going crazy

        // estimate sigma (same as FWHM or Gamma)
        double amplitude = histo.GetBinContent(bin_max_val);
        double sigma     = peak_volume / (amplitude * SQRT_2PI);
        fPeakFcn->SetParameter(2, sigma);
        fPeakFcn->SetParLimits(2, 0, sigma * 10.); // "reasonable" limits to prevent ROOT going crazy

        // fit in range, quiet mode, do not draw, use integral, improve result
        std::lock_guard<std::mutex> guard(fFitLock);
        TFitResultPtr               fitptr = histo.Fit(fPeakFcn.get(), "SRQMI0");
        // so that function will be drawn together with histo in the future
        histo.GetFunction(fPeakFcn->GetName())->ResetBit(TF1::kNotDraw);

        // calculate peak volume, set bcg to zero
        p0 = fPeakFcn->GetParameter(4);
        p1 = fPeakFcn->GetParameter(5);
        fPeakFcn->SetParameter(4, 0);
        fPeakFcn->SetParameter(5, 0);
        const double peak_volume_final = fPeakFcn->Integral(xmin, xmax);

        double width;
        if (gOptiFitterParameters.strategy == kDynamic_FWHM) width = EstimateFWxM(xmin, xmax, gOptiFitterParameters.strategy_value, 1);
        else
            width = EstimateWidth(xmin, xmax, gOptiFitterParameters.strategy_value, 1);
        fFitResults[0] = peak_volume_final;
        fFitResults[1] = fPeakFcn->GetParameter(1);
        fFitResults[2] = width;
        fFitResults[3] = fPeakFcn->GetParameter(3);

        // just in case
        fPeakFcn->SetParameter(4, p0);
        fPeakFcn->SetParameter(5, p1);

        return fitptr->IsValid();
    }

    bool FitPeak(TH1 &histo) override
    {
        double xmin, xmax;
        int    bin_min, bin_max;
        if (!fRangeSet)
        {
            xmin = histo.GetXaxis()->GetXmin();
            xmax = histo.GetXaxis()->GetXmax();

            bin_min = histo.FindBin(xmin);
            bin_max = histo.FindBin(xmax);

            fPeakFcn->SetRange(xmin, xmax);
        }
        else
        {
            xmin = fEnMin;
            xmax = fEnMax;

            bin_min = histo.FindBin(xmin);
            bin_max = histo.FindBin(xmax);

            fPeakFcn->SetRange(xmin, xmax);
        }

        histo.GetXaxis()->SetRangeUser(xmin, xmax);


        fPeakFcn->SetNpx((bin_max - bin_min) * 10);

        double y_min = histo.GetBinContent(bin_min);
        double y_max = histo.GetBinContent(bin_max);

        // estimate background
        double p1 = (y_max - y_min) / (xmax - xmin);
        double p0 = y_min - p1 * xmin;
        fPeakFcn->SetParameter(4, p0);
        fPeakFcn->SetParameter(5, p1);

        // set skewness to zero - assume regular gaussian first
        fPeakFcn->SetParameter(3, 0.);
        fPeakFcn->SetParLimits(3, 0., 100.);

        // estimate mean
        int bin_max_val = histo.GetMaximumBin();
        fPeakFcn->SetParameter(1, histo.GetBinCenter(bin_max_val));
        fPeakFcn->SetParLimits(1, xmin, xmax);

        // estimate volume - assuming regular Gaussian
        const double bcg_counts  = 0.5 * (xmax - xmin) * (p1 * (xmin + xmax) + 2. * p0);
        const double all_counts  = histo.Integral(bin_min, bin_max);
        const double peak_volume = all_counts - bcg_counts;
        assert(peak_volume > 0 && "estimated volume of Lorenzian appears to be negative");
        fPeakFcn->SetParameter(0, peak_volume);
        fPeakFcn->SetParLimits(0, all_counts / 100., all_counts * 100.); // reasonable limits to prevent ROOT going crazy

        // estimate sigma (same as FWHM or Gamma)
        double amplitude = histo.GetBinContent(bin_max_val);
        double sigma     = peak_volume / (amplitude * SQRT_2PI);
        fPeakFcn->SetParameter(2, sigma);
        fPeakFcn->SetParLimits(2, 0, sigma * 10.); // "reasonable" limits to prevent ROOT going crazy

        // fit in range, quiet mode, do not draw, use integral, improve result
        std::lock_guard<std::mutex> guard(fFitLock);
        TFitResultPtr               fitptr = histo.Fit(fPeakFcn.get(), "SRQMI0");
        // so that function will be drawn together with histo in the future
        histo.GetFunction(fPeakFcn->GetName())->ResetBit(TF1::kNotDraw);

        // calculate peak volume, set bcg to zero
        p0 = fPeakFcn->GetParameter(4);
        p1 = fPeakFcn->GetParameter(5);
        fPeakFcn->SetParameter(4, 0);
        fPeakFcn->SetParameter(5, 0);
        const double peak_volume_final = fPeakFcn->Integral(xmin, xmax);

        double width;
        if (gOptiFitterParameters.strategy == kDynamic_FWHM) width = EstimateFWxM(xmin, xmax, gOptiFitterParameters.strategy_value, 1);
        else
            width = EstimateWidth(xmin, xmax, gOptiFitterParameters.strategy_value, 1);
        fFitResults[0] = peak_volume_final;
        fFitResults[1] = fPeakFcn->GetParameter(1);
        fFitResults[2] = width;
        fFitResults[3] = fPeakFcn->GetParameter(3);

        // just in case
        fPeakFcn->SetParameter(4, p0);
        fPeakFcn->SetParameter(5, p1);

        return fitptr->IsValid();
    }
};

} // namespace gridopti