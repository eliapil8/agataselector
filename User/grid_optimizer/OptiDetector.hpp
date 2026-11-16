#pragma once

#include "TDirectory.h"
#include "TFile.h"
#include "TH1.h"
#include "TMath.h"
#include "TObject.h"
#include "TVector3.h"
#include <array>
#include <assert.h>
#include <string>
#include <vector>

#include "ConfManager.h"

#include "OptiSegment.hpp"
#include "TPositionOffset.hpp"
#include "misc.hh"
#include "OptiFitter.hpp"

namespace gridopti
{
// params[0] is the constant term
// params[1] is the coefficient for x
// params[2] is the coefficient for y
// params[3] is the coefficient for x^2
// params[4] is the coefficient for y^2
double poly2D(Double_t *x, Double_t *params)
{

    // return params[0] + params[1] * x[0] + params[2] * x[1] + params[3] * x[0] * x[0] + params[4] * x[1] * x[1];
    double retval=params[0];
    for(int i=1;i <5; i++)
    {
        retval += TMath::Power(x[0],i)*params[i*2-1];
        retval += TMath::Power(x[1],i)*params[i*2]; 
    }
    return retval;
}

/// @brief Virtual base class from which specific detector classes are to be derived to share same calling functions
class OptiDetector
{
  public:
    OptiDetector() = delete;
    OptiDetector(const std::string &name) : fOptiDetName(name){ this->ProcessConf();};
    virtual ~OptiDetector();

    /// @brief check if fits were already processed - if yes the file must exist and be in read-only mode
    /// @return
    bool AreFitsProcessed();

    /// @brief Manipulates the LUT based on the provided offset.
    /// @param offset A 3D vector representing the offset.
    virtual void ManipulateLUT(const TPositionOffset &offset) = 0;

    /// @brief Process root file processed by selector: find desirable matrix, slice it for each segment, fit energy of interest and store
    /// value.
    /// @param file_name name of the file including full path and name
    /// @param matrix_name name of the matrix, that has detector id on X axis (with its name as labels) and to-be-fitted spectrum on Y axis
    virtual void ProcessROOTFile(const std::string &file_name, const std::string &matrix_name) = 0;

    /// @brief Optimizes the detector based on the provided strategy.
    /// @param allow_invalid_fits should projection fits that returned invalid status be used in the optimization ?
    virtual void Optimize(const bool allow_invalid_fits = false) = 0;

    /// @brief Processes the configuration.
    void ProcessConf();

    /// @brief Gets the offsets.
    /// @return A pointer to a vector of 3D vectors representing the offsets.
    const std::vector<TPositionOffset> &GetOffsets() noexcept { return fOffsets; };

    /// @brief save optimized LUT to file at the same location as old one with "_optimized" suffix
    /// @param force_write_bad_fits
    virtual void SaveOptimizedLUT(const std::string &new_lut_path_and_name = "") = 0;

    /// @brief assign each detector appropriate bin number
    void SetBinNumbers(const std::string &file_name);

    /// @brief Similar to @ref ManipulateLUT, but it changes LUT values to based on optimal positions from the fit.
    /// Optimal position is initialized as a default position and is modified only after the fit is performed.
    virtual void OptimizeLUT() = 0;

    /// @brief Flushes the data stored in the detector segments into a file
    void FlushDataToFile()
    {
        std::for_each(fSegments.begin(), fSegments.end(), [](auto &det) { det.WriteToFile(); });
    }

    const std::string& GetName() const noexcept { return fOptiDetName;}

    static std::string GetMatrixUnit(const TH2 *matrix) noexcept;

  protected:

    /// @brief parse string from config file to get the fit function type
    /// @param function_name 
    /// @return 
    gridopti::optiFitterType GetFitterFunctionType(std::string function_name);


    gridopti::optiFitStrategy GetFitterStrategyType(std::string strategy_name);

    /// @brief Write all detector histogram fits to a given file
    /// @param file 
    void SaveFitsToFile(TFile &file);

    /// @brief Create a ROOT file that is distributed to all segments and used to store the results of the fits.
    /// @param filename 
    /// @param overwrite 
    void SetupDetectorFile(const std::string &filename, bool overwrite = false);

  protected:
    std::string                  fOptiDetName;
    std::vector<optiSegment>     fSegments;
    std::vector<TPositionOffset> fOffsets;
    std::shared_ptr<TFile>       fResultsFile;

  public:
    /// @brief Finds a histogram in a ROOT file.
    /// @param dir A pointer to the directory in the ROOT file.
    /// @param hist_name The name of the histogram.
    /// @return A pointer to the found histogram, or nullptr if not found
    static TObject *FindHistoInROOTFile(TDirectory *dir, const std::string &hist_name) noexcept;
};

void OptiDetector::SetupDetectorFile(const std::string &filename, bool overwrite)
{
    TDirectory *dir = gDirectory;
    // check if file exists
    fResultsFile = std::make_shared<TFile>(filename.c_str(), "READ");
    if(overwrite || !fResultsFile || fResultsFile->IsZombie() || fResultsFile->TestBit(TFile::kRecovered))
    {
        std::cout << "file " << filename << " does not exist or is corrupted, creating new one" << std::endl;
        std::remove(filename.c_str());
    }
    fResultsFile = std::make_shared<TFile>(filename.c_str(), "UPDATE");
    assert(fResultsFile && !fResultsFile->IsZombie());
    gDirectory = dir;
}

gridopti::optiFitterType OptiDetector::GetFitterFunctionType(std::string function_name)
{
    std::cout << "GetFitterFunctionType: " << function_name << std::endl;
    std::transform(function_name.begin(), function_name.end(), function_name.begin(), ::tolower);
    if (function_name == "skewedgaussian") return gridopti::optiFitterType::kSkewedGaussian;
    if (function_name == "lorenzian") return gridopti::optiFitterType::kLorenz;
    std::runtime_error("Unknown function type: " + function_name);
    return gridopti::optiFitterType::kVoid_type;
}

gridopti::optiFitStrategy OptiDetector::GetFitterStrategyType(std::string strategy_name)
{
    std::cout << "GetFitterStrategyType: " << strategy_name << std::endl;
    std::transform(strategy_name.begin(), strategy_name.end(), strategy_name.begin(), ::tolower);
    if (strategy_name == "fwxm") return gridopti::optiFitStrategy::kDynamic_FWHM;
    if (strategy_name == "static_value") return gridopti::optiFitStrategy::kStatic_WidthAtVal;
    std::runtime_error("Unknown strategy type: " + strategy_name);
    return gridopti::optiFitStrategy::kVoid_strategy;

}

OptiDetector::~OptiDetector()
{
    // this needs to be done, otherwise some process will try to access deleted memory, boh
    fSegments.clear();
}

void OptiDetector::SetBinNumbers(const std::string &file_name)
{
    ConfManager &conf = ConfManager::getInstance();

    TFile rfile(file_name.c_str(), "READ"); // open root file to change gDirectory
    auto *obj = FindHistoInROOTFile(gDirectory, conf.user.matrix_name);

    assert(obj && obj->InheritsFrom(TH2::Class()));
    TH2 *matrix = dynamic_cast<TH2 *>(obj);

    auto *axis = matrix->GetXaxis();

    for (auto bin = 1; bin < axis->GetNbins() - 1; bin++)
    {
        std::string label = axis->GetBinLabel(bin);
        // try to find bin label in our detectors
        for (auto &det : fSegments)
        {
            if (det.GetLabelName().compare(axis->GetBinLabel(bin)) != 0) continue;
            det.SetBinNumber(bin);
        }
    }
    rfile.Close();
}

bool OptiDetector::AreFitsProcessed()
{
    assert(fResultsFile);
    if (fResultsFile->IsWritable()) return false;
    return true;
}

void OptiDetector::SaveFitsToFile(TFile &file)
{
    file.cd();
    assert(file.IsWritable());

    // lets be civilized and reorder the detectors by their label name
    std::vector<size_t> indices(fSegments.size());
    std::iota(indices.begin(), indices.end(), 0);

    std::sort(indices.begin(), indices.end(),
              [&](size_t a, size_t b) { return fSegments[a].GetLabelName() < fSegments[b].GetLabelName(); });

    for (auto &index : indices)
    {
        auto &det = fSegments[index];

        file.cd();
        TDirectory *dir = dynamic_cast<TDirectory *>(file.Get(det.GetLabelName().c_str()));
        if (!dir) { dir = file.mkdir(det.GetLabelName().c_str()); }
        // dir->cd();
        det.Write(dir);
        // file.cd("..");
    }
}

std::string OptiDetector::GetMatrixUnit(const TH2 *matrix) noexcept
{
    assert(matrix);
    std::string axis_label = matrix->GetYaxis()->GetTitle();

    std::size_t start = axis_label.find('[');
    std::size_t end   = axis_label.find(']');

    if (start != std::string::npos && end != std::string::npos)
    {
        return axis_label.substr(start + 1, end - start - 1); // Extract the substring between '[' and ']'
    }
    else
    {
        return ""; // If no match found, return an empty string
    }
}

void OptiDetector::ProcessConf()
{
    ConfManager &conf_main = ConfManager::getInstance();
    UserConf &   conf      = conf_main.user;

    // set global setting for the fitter
    auto fcn_type = this->GetFitterFunctionType(conf.peak_fit_function);
    auto fcn_strategy = this->GetFitterStrategyType(conf.peak_fit_width_strategy);
    auto fcn_strategy_value = conf.peak_fit_width_strategy_value;
  
    gOptiFitterParameters.type = fcn_type;
    gOptiFitterParameters.strategy = fcn_strategy;
    gOptiFitterParameters.strategy_value = fcn_strategy_value;


    // settings for setting the offsets
    if (!conf.run_selector_loop) return;

    // START doing sanity checks and define 0 values when some parts are missing from conf file
    bool using_spherical_range{false};
    bool using_spherical_range_theta{false};
    bool using_spherical_range_phi{false};
    bool using_spherical_custom{false};
    bool using_carthesian_range{false};
    bool using_carthesian_range_x{false};
    bool using_carthesian_range_y{false};
    bool using_carthesian_range_z{false};
    bool using_carthesian_custom{false};

    // PROCESS REQUIREMETS
    if (conf.matrix_name.empty()) std::runtime_error("MATRIX_NAME is not set in the configuration file!");

    // PROCESS SPHERICAL COORDINATES
    if (conf.theta_offset_range_start != std::numeric_limits<double>::infinity() &&
        conf.theta_offset_range_end != std::numeric_limits<double>::infinity() &&
        conf.theta_step != std::numeric_limits<double>::infinity())
    {
        using_spherical_range       = true;
        using_spherical_range_theta = true;
    }

    if (conf.phi_offset_range_start != std::numeric_limits<double>::infinity() &&
        conf.phi_offset_range_end != std::numeric_limits<double>::infinity() && conf.phi_step != std::numeric_limits<double>::infinity())
    {
        using_spherical_range     = true;
        using_spherical_range_phi = true;
    }

    if (using_spherical_range && !using_spherical_range_theta)
    {
        conf.theta_offset_range_start = 0.0;
        conf.theta_offset_range_end   = 0.0;
        conf.theta_step               = 0.0;
    }

    if (using_spherical_range && !using_spherical_range_phi)
    {
        conf.phi_offset_range_start = 0.0;
        conf.phi_offset_range_end   = 0.0;
        conf.phi_step               = 0.0;
    }

    if (conf.theta_custom_offset.size() != 0 || conf.phi_custom_offset.size() != 0)
    {
        if (conf.theta_custom_offset.size() == conf.phi_custom_offset.size()) { using_spherical_custom = true; }
        else
        {
            std::runtime_error("number of elements in THETA_CUSTOM_OFFSET and PHI_CUSTOM_OFFSET is not identical!");
        }
    }

    // PROCESS CARTHESIAN COORDINATES
    if (conf.x_offset_range_start != std::numeric_limits<double>::infinity() &&
        conf.x_offset_range_end != std::numeric_limits<double>::infinity() && conf.x_step != std::numeric_limits<double>::infinity())
    {
        using_carthesian_range   = true;
        using_carthesian_range_x = true;
    }
    if (conf.y_offset_range_start != std::numeric_limits<double>::infinity() &&
        conf.y_offset_range_end != std::numeric_limits<double>::infinity() && conf.y_step != std::numeric_limits<double>::infinity())
    {
        using_carthesian_range   = true;
        using_carthesian_range_y = true;
    }
    if (conf.z_offset_range_start != std::numeric_limits<double>::infinity() &&
        conf.z_offset_range_end != std::numeric_limits<double>::infinity() && conf.z_step != std::numeric_limits<double>::infinity())
    {
        using_carthesian_range   = true;
        using_carthesian_range_z = true;
    }

    if (using_carthesian_range && !using_carthesian_range_x)
    {
        conf.x_offset_range_start = 0.0;
        conf.x_offset_range_end   = 0.0;
        conf.x_step               = 0.0;
    }

    if (using_carthesian_range && !using_carthesian_range_y)
    {
        conf.y_offset_range_start = 0.0;
        conf.y_offset_range_end   = 0.0;
        conf.y_step               = 0.0;
    }

    if (using_carthesian_range && !using_carthesian_range_z)
    {
        conf.z_offset_range_start = 0.0;
        conf.z_offset_range_end   = 0.0;
        conf.z_step               = 0.0;
    }

    if (conf.x_custom_offset.size() != 0 || conf.y_custom_offset.size() != 0 || conf.z_custom_offset.size() != 0)
    {
        if (conf.x_custom_offset.size() == conf.y_custom_offset.size() && conf.x_custom_offset.size() == conf.z_custom_offset.size())
        { using_spherical_custom = true; }
        else
        {
            std::runtime_error("number of elements in X_CUSTOM_OFFSET, Y_CUSTOM_OFFSET and Z_CUSTOM_OFFSET is not identical!");
        }
    }

    if (!(using_spherical_range || using_spherical_custom || using_carthesian_range || using_carthesian_custom))
    { std::runtime_error("No offsets were set up in the configuration file!"); }

    if (using_spherical_range)
    {
        assert(conf.theta_offset_range_end >= conf.theta_offset_range_start &&
               "theta_offset_range_start must be smaller then theta_offset_range_end");
        assert(conf.phi_offset_range_end >= conf.phi_offset_range_start &&
               "phi_offset_range_start must be smaller then phi_offset_range_end");
    }

    if (using_carthesian_range)
    {
        assert(conf.x_offset_range_end >= conf.x_offset_range_start && "x_offset_range_start must be smaller then x_offset_range_end");
        assert(conf.y_offset_range_end >= conf.y_offset_range_start && "y_offset_range_start must be smaller then y_offset_range_end");
        assert(conf.z_offset_range_end >= conf.z_offset_range_start && "z_offset_range_start must be smaller then z_offset_range_end");
    }

    // END OF sanity checks

    if (using_spherical_range)
    {
        std::vector<double> theta_angles;
        std::vector<double> phi_angles;
        double              angle;

        // calculate theta angles
        if (conf.theta_offset_range_start <= 0. && conf.theta_offset_range_end >= 0.)
        {
            angle = 0.;
            // 0 and negative angles
            while (angle >= conf.theta_offset_range_start)
            {
                theta_angles.emplace_back(angle);
                angle -= conf.theta_step;
            }
            angle = 0.;
            // 0 and positive angles
            while (angle <= conf.theta_offset_range_end)
            {
                theta_angles.emplace_back(angle);
                angle += conf.theta_step;
            }
        }
        else
        {
            angle = conf.theta_offset_range_start;
            while (angle <= conf.theta_offset_range_end)
            {
                theta_angles.emplace_back(angle);
                angle += conf.theta_step;
            }
        }

        // calculate phi angles
        if (conf.phi_offset_range_start <= 0. && conf.phi_offset_range_end >= 0.)
        {
            angle = 0.;
            // 0 and negative angles
            while (angle >= conf.phi_offset_range_start)
            {
                phi_angles.emplace_back(angle);
                angle -= conf.phi_step;
            }
            angle = 0.;
            // 0 and positive angles
            while (angle <= conf.phi_offset_range_end)
            {
                phi_angles.emplace_back(angle);
                angle += conf.phi_step;
            }
        }
        else
        {
            angle = conf.phi_offset_range_start;
            while (angle <= conf.phi_offset_range_end)
            {
                phi_angles.emplace_back(angle);
                angle += conf.phi_step;
            }
        }

        for (const auto t : theta_angles)
        {
            for (const auto p : phi_angles)
            {
                TPositionOffset offset(t, p);
                fOffsets.emplace_back(std::move(offset));
            }
        }
    }
    if (using_spherical_custom)
    {
        for (const auto t : conf.theta_custom_offset)
        {
            for (const auto p : conf.phi_custom_offset)
            {
                TPositionOffset offset(t * TMath::DegToRad(), p * TMath::DegToRad());
                fOffsets.emplace_back(std::move(offset));
            }
        }
    }
    if (using_carthesian_range)
    {
        std::vector<double> x_pos;
        std::vector<double> y_pos;
        std::vector<double> z_pos;
        double              pos;

        // calculate x positions
        if (conf.x_offset_range_start <= 0. && conf.x_offset_range_end >= 0.)
        {
            pos = 0.;
            // 0 and negative poss
            while (pos >= conf.x_offset_range_start)
            {
                x_pos.emplace_back(pos);
                pos -= conf.x_step;
            }
            pos = 0.;
            // 0 and positive poss
            while (pos <= conf.x_offset_range_end)
            {
                x_pos.emplace_back(pos);
                pos += conf.x_step;
            }
        }
        else
        {
            pos = conf.x_offset_range_start;
            while (pos <= conf.x_offset_range_end)
            {
                x_pos.emplace_back(pos);
                pos += conf.x_step;
            }
        }

        // calculate y positions
        if (conf.y_offset_range_start <= 0. && conf.y_offset_range_end >= 0.)
        {
            pos = 0.;
            // 0 and negative poss
            while (pos >= conf.y_offset_range_start)
            {
                y_pos.emplace_back(pos);
                pos -= conf.y_step;
            }
            pos = 0.;
            // 0 and positive poss
            while (pos <= conf.y_offset_range_end)
            {
                y_pos.emplace_back(pos);
                pos += conf.y_step;
            }
        }
        else
        {
            pos = conf.y_offset_range_start;
            while (pos <= conf.y_offset_range_end)
            {
                y_pos.emplace_back(pos);
                pos += conf.y_step;
            }
        }

        // calculate z positions
        if (conf.z_offset_range_start <= 0. && conf.z_offset_range_end >= 0.)
        {
            pos = 0.;
            // 0 and negative poss
            while (pos >= conf.z_offset_range_start)
            {
                z_pos.emplace_back(pos);
                pos -= conf.z_step;
            }
            pos = 0.;
            // 0 and positive poss
            while (pos <= conf.z_offset_range_end)
            {
                z_pos.emplace_back(pos);
                pos += conf.z_step;
            }
        }
        else
        {
            pos = conf.z_offset_range_start;
            while (pos <= conf.z_offset_range_end)
            {
                z_pos.emplace_back(pos);
                pos += conf.z_step;
            }
        }

        for (const auto x : x_pos)
        {
            for (const auto y : y_pos)
            {
                for (const auto z : z_pos)
                {
                    TPositionOffset offset(x, y, z);
                    fOffsets.emplace_back(std::move(offset));
                }
            }
        }
    }
    if (using_carthesian_custom)
    {
        for (const auto x : conf.x_custom_offset)
        {
            for (const auto y : conf.y_custom_offset)
            {
                for (const auto z : conf.z_custom_offset)
                {
                    TPositionOffset offset(x, y, z);
                    fOffsets.emplace_back(std::move(offset));
                }
            }
        }
    }

    // remove duplicates
    // fOffsets.erase(std::unique(fOffsets.begin(), fOffsets.end()), fOffsets.end()); //this does not work because unique requires sorted
    // vector and we cannot defined comparison operators
    std::vector<TPositionOffset> temp;
    for (auto &offset : fOffsets)
    {
        auto it = std::find(temp.begin(), temp.end(), offset);
        if (it == temp.end()) temp.emplace_back(offset);
    }
    fOffsets = std::move(temp);
    
    // PrintInfo_normal("Following offsets of [theta,phi] will be computed:");
    // for (const auto &v : fOffsets) { v.Print(); }
}

TObject *OptiDetector::FindHistoInROOTFile(TDirectory *dir, const std::string &hist_name) noexcept
{
    TIter nextkey(dir->GetListOfKeys());
    TKey *key;

    while ((key = dynamic_cast<TKey *>(nextkey())))
    {
        TObject *obj = key->ReadObj();
        // Check if the key is a subdirectory
        if (obj->IsA()->InheritsFrom(TDirectory::Class()))
        {
            TDirectory *subdir = dynamic_cast<TDirectory *>(obj);

            // Recursively search in the subdirectory
            TObject *result = FindHistoInROOTFile(subdir, hist_name);
            if (result) { return result; }
        }
        else
        {
            // Check if the key is the desired object
            if (key->GetName() == hist_name) return obj;
        }
        delete obj;
    }
    return nullptr;
}

} // namespace gridopti