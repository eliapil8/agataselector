#pragma once

#include "TDirectory.h"
#include "TFile.h"
#include "TH1.h"
#include "TMath.h"
#include "TObject.h"
#include "TTree.h"
#include "TVector3.h"
#include <array>
#include <assert.h>
#include <mutex>
#include <string>
#include <vector>

#include "OptiFitter.hpp"
#include "TOptiData.hpp"
#include "TPositionOffset.hpp"

namespace gridopti
{

/// @brief This class serves to describe and store details about individual segment/pixel that is being optimized.
class optiSegment
{
  private:
    /// @brief from LUT
    std::string fName;
    /// @brief from LUT
    int fBoard;
    /// @brief from LUT
    int fChannel;
    /// @brief from LUT
    TVector3 fPosition_original;
    /// @brief storing optimal position
    TVector3 fPosition_optimal;
    /// @brief  flag to indicate if the fitter succeeded to find minimum in the global plane
    bool fIsOptimizedGood{false};
    /// @brief bin number for quick access
    int fBinNumer{-1};
    /// @brief label name used in the matrix of X:Y = detector_label : energy_spectrum (one containing peak to be minimized)
    std::string fLabelName;
    double      fEnPeak, fEnMin, fEnMax;

    /// @brief tfile to which the fit histograms are written
    std::shared_ptr<TFile> fFile;
    /// @brief directory in a fFile to which the histograms are written
    TDirectory *fROOTdir;
    /// @brief variable used to read and write to a tree
    TOptiData *fData{nullptr};
    /// @brief tree containing the fit results
    std::unique_ptr<TTree> fDataTree;

    /// @brief fitter used to fit the matrix projections
    std::unique_ptr<OptiFitter> fFitter{nullptr};
    static std::mutex           fFileAccessMutex;

  public:
    optiSegment() = delete;
    optiSegment(const std::string &name, const int board, const int channel, const TVector3 &position, std::shared_ptr<TFile> file)
        : fName(name), fBoard(board), fChannel(channel), fPosition_original(position), fPosition_optimal(position), fFile(std::move(file))
    {
        std::lock_guard<std::mutex> lock(fFileAccessMutex);

        TDirectory *current_dir = gDirectory;
        fData                   = new TOptiData();
        assert(fFile && fFile->IsOpen() && "File is not open or does not exist");
        if (fFile->IsWritable())
        {
            fROOTdir = dynamic_cast<TDirectory *>(fFile->Get(name.c_str()));
            if(!fROOTdir)
            {
                fROOTdir  = fFile->mkdir(name.c_str());
            }
            fDataTree = std::make_unique<TTree>(fName.c_str(), fName.c_str());
            fDataTree->SetDirectory(fROOTdir);
            fDataTree->Branch("data", fData);
            assert(fDataTree && "Tree does not exist");
        }
        else
        {
            fROOTdir = dynamic_cast<TDirectory *>(fFile->Get(name.c_str()));
            assert(fROOTdir && "Directory does NOT exist");
            fDataTree = std::unique_ptr<TTree>(dynamic_cast<TTree *>(fROOTdir->Get(fName.c_str())));
            if (!fDataTree)
                std::runtime_error("Tree does NOT exist, probably the file is corrupted, try removing file " +
                                   (std::string)fFile->GetName() + " and rerun");
            fDataTree->SetDirectory(fROOTdir);
            fDataTree->SetBranchAddress("data", &fData);
        }

        if (gOptiFitterParameters.type == kSkewedGaussian) { fFitter = std::make_unique<OptiFitterSkewedGaussian>(); }
        else if (gOptiFitterParameters.type == kLorenz)
        {
            fFitter = std::make_unique<OptiFitterLorenz>();
        }
        else
        {
            std::runtime_error("Unknown fit function type");
        }

        this->SetLabelName(name);
        gDirectory = current_dir;
    }

    // Copy constructor
    optiSegment(const optiSegment &other) = delete;

    // Move constructor
    optiSegment(optiSegment &&other) noexcept
        : fName(std::move(other.fName)), fBoard(std::move(other.fBoard)), fChannel(std::move(other.fChannel)),
          fPosition_original(std::move(other.fPosition_original)), fPosition_optimal(std::move(other.fPosition_optimal)),
          fIsOptimizedGood(std::move(other.fIsOptimizedGood)), fBinNumer(std::move(other.fBinNumer)),
          fLabelName(std::move(other.fLabelName)), fEnPeak(std::move(other.fEnPeak)), fEnMin(std::move(other.fEnMin)),
          fEnMax(std::move(other.fEnMax)), fFile(std::move(other.fFile)), fROOTdir(std::move(other.fROOTdir)),
          fData(std::move(other.fData)), fDataTree(std::move(other.fDataTree)), fFitter(std::move(other.fFitter))
    {
        other.fROOTdir  = nullptr;
        other.fDataTree = nullptr;
        other.fFile     = nullptr;
        other.fData     = nullptr;
        other.fFitter   = nullptr;
    }

    ~optiSegment() { delete fData; }

    void WriteToFile()
    {
        if (fROOTdir && fDataTree && fFile)
        {
            if (fDataTree->GetEntriesFast() != 0 && fFile->IsWritable())
            {
                TDirectory *current_dir = gDirectory;
                fROOTdir->cd();
                fDataTree->Write();
                // fDataTree->SetDirectory(0); // already written in a file. must be detached to prevent double writing
                gDirectory = current_dir;
            }
        }
    }

    /// @brief set optimal position of the detector segment
    /// @param theta_offset expected to be in radians
    /// @param phi_offset expected to be in radians
    void SetOptimal_position(const double &theta_offset, const double &phi_offset, bool is_good_fit)
    {
        fIsOptimizedGood = is_good_fit;
        // if (fPosition_original.Theta() + theta_offset < (180. * TMath::DegToRad()))
        // {
        //     theta_offset = fPosition_original.Theta() - (180. * TMath::DegToRad());
        //     phi_offset += (180. * TMath::DegToRad());
        // }

        fPosition_optimal = fPosition_original;
        fPosition_optimal.SetTheta(fPosition_optimal.Theta() + theta_offset);
        fPosition_optimal.SetPhi(fPosition_optimal.Phi() + phi_offset);

    }

    /// @brief set Xbin number of the matrix that contain the (projected) histogram for this segment
    void SetBinNumber(int bin_number) noexcept { fBinNumer = bin_number; }
    /// @brief get Xbin number of the matrix that contain the (projected) histogram for this segment
    int GetBinNumber() const noexcept { return fBinNumer; }

    /// @brief set optimal position of the detector segment
    /// @param dx expected in same unit as position vector
    /// @param dy expected in same unit as position vector
    /// @param dz expected in same unit as position vector
    void SetOptimal_position(const double &dx, const double &dy, const double &dz, bool is_good_fit)
    {
        fIsOptimizedGood  = is_good_fit;
        fPosition_optimal = fPosition_original;
        fPosition_optimal.SetX(fPosition_original.X() + dx);
        fPosition_optimal.SetY(fPosition_original.Y() + dy);
        fPosition_optimal.SetZ(fPosition_original.Z() + dz);
    }

    /// @brief write data to current directory
    void Write(TDirectory *dir)
    {
        TDirectory *current_dir = gDirectory;
        dir->cd();
        std::lock_guard<std::mutex> lock(fFileAccessMutex);
        for (Long64_t entry = 0; entry < this->GetDataSize(); ++entry)
        {
            fDataTree->GetEntry(entry);
            // std::string hname{fDataRead->GetName()};
            std::string hname{fData->GetName()};
            // hname += "_" + fDataRead->GetPosOffset().GenerateString();
            hname += "_" + fData->GetPosOffset().GenerateString();
            TH1 *temp = dynamic_cast<TH1 *>(fROOTdir->Get(hname.c_str()));
            if (!temp) continue;
            temp->SetDirectory(dir);
            dir->cd();
            temp->Write();
            temp->SetDirectory(0);
            delete temp;
        }
        gDirectory = current_dir;
    }

    /// @brief set parameters of the peak that is to be minimized
    /// @param en_peak energy centroid
    /// @param en_min fitting range min
    /// @param en_max fitting range max
    void SetPeak(const double &en_peak, const double &en_min, const double &en_max)
    {
        fEnPeak = en_peak;
        fEnMin  = en_min;
        fEnMax  = en_max;
    }

    Long64_t         GetDataSize() const { return fDataTree->GetEntriesFast(); }
    const TOptiData &GetData(const Long64_t &i) const
    {
        fDataTree->GetEntry(i);
        return *fData;
    }

    /// @brief reset internal pointer to point at the beginning of the container holding fit result data
    void               SetLabelName(std::string label_name) noexcept { fLabelName = label_name; };
    const std::string &GetLabelName() const noexcept { return fLabelName; };
    const double &     GetEnergy_Peak() const noexcept { return fEnPeak; };
    const double &     GetEnergy_Min() const noexcept { return fEnMin; };
    const double &     GetEnergy_Max() const noexcept { return fEnMax; };
    void               GetBoardChannel(int &board, int &channel) const noexcept
    {
        board   = fBoard;
        channel = fChannel;
    }

    const TVector3 &GetOriginalPosition() const noexcept { return fPosition_original; }
    const TVector3 &GetOptimalPosition() const noexcept { return fPosition_optimal; }

    /// @brief get optimal position of the detector segment
    /// @param optimal_pos optimal vector as produced by fit
    /// @return true if fit was successful, false otherwise
    bool GetOptimizedPosition(TVector3 &optimal_pos)
    {
        optimal_pos = fPosition_optimal;
        return fIsOptimizedGood;
    }

    void SaveObjectToFile(TFile *file, TObject &&obj)
    {
        assert(file);
        file->cd();
        TDirectory *dir = (TDirectory *)file->Get(this->GetLabelName().c_str());
        if (!dir) { dir = file->mkdir(this->GetLabelName().c_str()); }
        dir->cd();
        obj.Write();
        file->cd("..");
    }

    void SaveObjectToFile(TFile *file, TObject *obj)
    {
        TDirectory* orig_dir = gDirectory;
        assert(file);
        file->cd();
        TDirectory *dir = (TDirectory *)file->Get(this->GetLabelName().c_str());
        if (!dir) { dir = file->mkdir(this->GetLabelName().c_str()); }
        dir->cd();
        obj->Write();
        gDirectory = orig_dir;
    }

  public:
    /// @brief Function takes ownership of the projected histogram
    /// @param histo
    /// @param offset
    void ProcessProjection(TH1 *histo, const TPositionOffset *offset)
    {
        assert(histo);
        bool                        is_valid = fFitter->FitPeak(*histo, fEnMin, fEnMax);
        const std::array<double, 5> fit_res{fFitter->GetFitResults()};

        const std::string data_name = fName + "_" + offset->GenerateString();
        fData->SetData(data_name, *offset, fit_res, is_valid);
        // fData.SetData(data_name, *offset, fit_res, is_valid);
        histo->SetName(data_name.c_str());
        fFile->cd();
        histo->SetDirectory(fROOTdir);
        std::lock_guard<std::mutex> lock(fFileAccessMutex);
        fDataTree->Fill();

        // fDataTree->GetEntry(fDataTree->GetEntriesFast() - 1);
        // fData->Print();

        fROOTdir->cd();
        histo->Write();
        histo->SetDirectory(0); // must be detached to prevent double writing
        delete histo;
    }

    bool OffsetAlreadyProcessed(const TPositionOffset &offset)
    {
        for (Long64_t entry = 0; entry < this->GetDataSize(); ++entry)
        {
            fDataTree->GetEntry(entry);
            if (fData->GetPosOffset() == offset) return true;
        }
        return false;
    }

    void SetEnergyUnitCorrection(const double &correction_factor)
    {
        fEnPeak *= correction_factor;
        fEnMin *= correction_factor;
        fEnMax *= correction_factor;

        fFitter->SetRange(fEnMin, fEnMax);
    }
};

std::mutex gridopti::optiSegment::fFileAccessMutex;

} // namespace gridopti