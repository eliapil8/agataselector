#pragma once

#include <array>
#include <iterator>
#include <map>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

// #include "ROOT/TThreadExecutor.hxx"
#include "TDirectory.h"
#include "TF1.h"
#include "TF2.h"
#include "TGraph.h"
#include "TGraph2D.h"
#include "TH2.h"
#include "TPaveStats.h"

#include "ConfManager.h"
#include "Settings.h"

#include "OptiDetector.hpp"
#include "OptiSegment.hpp"
#include "TPositionOffset.hpp"
#include "calibrator_misc.hpp"
#include "misc.hh"

namespace gridopti
{

class OptiSpider : public OptiDetector
{
  public:
    OptiSpider();
    ~OptiSpider() override{}; //{std::cout << "OptiSpider destructor called" << std::endl;};
    /// @brief Perform fit of all accumulated points
    /// @param allow_invalid_fits should projection fits that returned invalid status be used in the optimization ?
    void Optimize(const bool allow_invalid_fits = false) override;

    /// @brief Reload LUT from the file and effectively remove all previous LUT changes
    inline void ReloadLUT();

    /// @brief Manipulate selector's lookup table values by offsetting current values stored in LUT
    /// @param offset offset to be applied to all values in LUT
    void ManipulateLUT(const TPositionOffset &offset) override;
    /// @brief Process root file processed by selector: find desirable matrix, slice it for each segment, fit energy of interest and store
    /// value
    /// @param file_name name of the file including full path and name
    /// @param matrix_name name of the matrix, that has detector id on X axis (with its name as labels) and to-be-fitted spectrum on Y axis
    void ProcessROOTFile(const std::string &file_name, const std::string &matrix_name) override;

    /// @brief Save optimized LUT to file
    void SaveOptimizedLUT(const std::string &new_lut_path_and_name = "") override;

    /// @brief Print debug information about LUT
    void PrintDebug();

    void OptimizeLUT() override;

  private:
    // variables for quick access to avoid calling ConfManager all the time
    SpiderLut * fLUT;
    SpiderConf *fConf;
    std::string fLut_path;

    std::unordered_map<unsigned int, std::unordered_map<unsigned int, SpiderLut::Properties>> *fLUTProp;

    void SpawnSegments();
};

// OptiSpider::~OptiSpider()
// {
//     OptiDetector::~OptiDetector();
// }

OptiSpider::OptiSpider() : OptiDetector("SPIDER")
{
    // setup some variables for quick access
    ConfManager &conf = ConfManager::getInstance();

    size_t      condition_hash  = std::hash<std::string>{}(conf.user.data_reduction_condition);
    std::string output_filename = OPTIMIZED_FILE_PREFIX + "processed_" + fOptiDetName  + '_' + std::to_string(condition_hash) + ".root";
    this->SetupDetectorFile(output_filename, false);

    fLUT      = &conf.spider.lut;
    fLUTProp  = &fLUT->GetProperties();
    fLut_path = conf.replay.conf_path + "/LUT/" + conf.spider.lutFile;

    this->SpawnSegments();
    // fLUT->PrintLut_new(std::cout);
}

void OptiSpider::SaveOptimizedLUT(const std::string &new_lut_path_and_name)
{
    this->OptimizeLUT();
    std::string new_lut_filename;
    if (new_lut_path_and_name.empty()) new_lut_filename = fLut_path + ".optimized";
    else
        new_lut_filename = new_lut_path_and_name;
    std::fstream file(new_lut_filename, std::ios::out);
    fLUT->PrintLut(file);

    PrintInfo_important("Optimized LUT saved to " + new_lut_filename);
    // PrintInfo_important("This is a new LUT:");
    // fLUT->PrintLut(std::cout);
}

void OptiSpider::PrintDebug()
{
    int seg_counter = 0;
    for (auto it1 = fLUTProp->begin(); it1 != fLUTProp->end(); it1++) { seg_counter += it1->second.size(); }
    std::cout << "Lut contains  " << seg_counter << " individual segments" << std::endl;
    for (auto it1 = fLUTProp->begin(); it1 != fLUTProp->end(); it1++)
    {
        auto _board = it1->first;
        auto _map   = it1->second;
        for (auto it2 = _map.begin(); it2 != _map.end(); it2++)
        {
            auto _channel = it2->first;
            auto _pos     = it2->second.pos;

            std::cout << "Board " << _board << " Channel " << _channel << " ";
            _pos.Print();
        }
    }
}

inline void OptiSpider::ReloadLUT()
{
    fLUTProp->clear();
    fLUT->ReadLut(fLut_path);
}

void OptiSpider::ProcessROOTFile(const std::string &file_name, const std::string &matrix_name)
{
    TFile rfile(file_name.c_str(), "READ"); // open root file to change gDirectory

    const TPositionOffset *pos = dynamic_cast<TPositionOffset *>(rfile.Get("position_offset"));
    assert(pos && "Could not find position_offset object in the file");
    // check if entry for this offset already exists in a tree
    if(fSegments.at(0).OffsetAlreadyProcessed(*pos)) return;

    auto *obj = FindHistoInROOTFile(gDirectory, matrix_name);
    assert(obj && obj->InheritsFrom(TH2::Class()));
    TH2 *matrix = dynamic_cast<TH2 *>(obj);

    // we need to get energy scale of the matrix from the Y label in order to match the scale used in the fit details (peak, min, max)
    // default unit is MeV
    static bool run_once = [&]() {
        // we need to get energy scale of the matrix from the Y label in order to match the scale used in the fit details (peak, min, max)
        // default unit is MeV
        std::string matrix_units    = GetMatrixUnit(matrix);
        double      unit_correction = !matrix_units.empty() ? 1. / UNITS::classify(matrix_units) : 1.;
        std::for_each(fSegments.begin(), fSegments.end(), [&](optiSegment &det) { det.SetEnergyUnitCorrection(unit_correction); });
        return true;
    }();
    (void)run_once; // Suppress unused variable warning

    std::vector<optiSegment *> segments;
    for (auto &det : fSegments) segments.emplace_back(&det);

    // cannot use, because ROOT is not thread safe for fitting and using forks here would be complicated :(
    // ROOT::TThreadExecutor pool;
    // auto empty_return = pool.Map([&]( gridopti::optiSegment* det)
    // {
    //     int bin = det->GetBinNumber();
    //     TH1D *      projection = matrix->ProjectionY(det->GetLabelName().c_str(), bin, bin);
    //     assert(projection);
    //     det->ProcessProjection(projection, pos);
    //     return 0;
    // }, segments);


    for (auto &det : fSegments)
    {
        int bin = det.GetBinNumber();
        // std::cout << "Processing " << det.GetLabelName() << " in " << file_name << '\r' << std::flush;

        TH1D *projection = matrix->ProjectionY(det.GetLabelName().c_str(), bin, bin);
        assert(projection);
        det.ProcessProjection(projection, pos);
    }
    delete pos;
    pos = nullptr;

    matrix->SetDirectory(nullptr);
    delete matrix;
    matrix = nullptr;
    rfile.Close();
}

void OptiSpider::Optimize(const bool allow_invalid_fits)
{
    UserConf &confu = ConfManager::getInstance().user;
    assert(confu.fit_weight_position >= 0. && confu.fit_weight_width >= 0. && confu.fit_weight_volume >= 0. &&
           confu.fit_weight_volume + confu.fit_weight_position + confu.fit_weight_width > 0. &&
           "weights for minimization must be positive and at least one of them must be non-zero");

    std::string savefile_name = confu.optiresult_file_path;
    if (savefile_name.back() != '/') savefile_name += "/";
    savefile_name += OPTIMIZED_FILE_PREFIX + "graph_fits_" + fOptiDetName + std::string(allow_invalid_fits ? "_only_valid_fits" : "_allow_invalid_fits") + ".root";
    PrintInfo_important("saving results to " + savefile_name);

    TFile savefile(savefile_name.c_str(), "RECREATE");
    this->SaveFitsToFile(savefile);

    double x;
    double y;
    double val{0.};
    for (auto &det : fSegments)
    {
        TGraph2D    gr;
        std::string gr_name = det.GetLabelName() + "_minimization_plane";
        if (!allow_invalid_fits) gr_name += "_only_valid_fits";
        else
            gr_name += "_allow_invalid_fits";

        gr.SetName(gr_name.c_str());
        gr.SetTitle(gr_name.c_str());

        auto nentries = det.GetDataSize();

        for (Long64_t entry = 0; entry < nentries; entry++)
        {
            auto data{det.GetData(entry)};
            x = data.GetPosOffset().GetThetaOffset() * TMath::RadToDeg();
            y = data.GetPosOffset().GetPhiOffset() * TMath::RadToDeg();
            if (!allow_invalid_fits && !data.IsValid()) continue;
            val = data.GetFitResult()[0] * confu.fit_weight_volume;
            val += abs(data.GetFitResult()[1] - confu.fit_peak_energy) * confu.fit_weight_position;
            val += data.GetFitResult()[2] * confu.fit_weight_width;
            val += data.GetFitResult()[3] * confu.fit_weight_par3;
            val += data.GetFitResult()[4] * confu.fit_weight_par4;
            gr.AddPoint(x, y, val);
        }
        // fit graph
        assert(gr.GetN() >= 5 && "graph must contain at least 5 point to be fitted with 2D polynomial of second order with 5 parameters");
        TF2 plane_fcn("plane_fcn", poly2D, gr.GetXmin(), gr.GetXmax(), gr.GetYmin(), gr.GetYmax(), 9, 2);
        for (int i = 0; i < plane_fcn.GetNpar(); i++) plane_fcn.SetParameter(i, 0.);
        PrintInfo_important("Fitting " + gr_name);

        TFitResultPtr fitptr = gr.Fit(&plane_fcn, "SWEM0QR");
        gr.ResetBit(TF2::kNotDraw);

        if (!fitptr->IsValid())
        {
            std::cout << '\n';
            PrintWarning(" FIT of tgraph " + std::string(gr.GetName()) + " IS NOT VALID");
        }
        TH2D* gr_matrix = gr.GetHistogram();
        gr_matrix->GetXaxis()->SetTitle("theta offset [deg]");
        gr_matrix->GetYaxis()->SetTitle("phi offset [deg]");
        gr_matrix->GetZaxis()->SetTitle("minimization value [arb.]");
        // save graph to file
        // det.SaveObjectToFile(&savefile, std::move(gr));
        TCanvas c1("fit_fcn_with_graph2d_points", "fit_fcn_with_graph2d_points", 800, 800);
        plane_fcn.Draw("surf1");
        gr.Draw("p0 same");

        det.SaveObjectToFile(&savefile, &c1);
        det.SaveObjectToFile(&savefile, gr_matrix);
        det.SaveObjectToFile(&savefile, &gr);
        det.SaveObjectToFile(&savefile, &plane_fcn);
        
        
        double minx, miy, val;
        plane_fcn.GetMinimumXY(minx, miy);
        val = plane_fcn.Eval(minx, miy);
        if (minx < gr.GetXmin() || minx > gr.GetXmax() || miy < gr.GetYmin() || miy > gr.GetYmax())
        {
            PrintWarning("Minimum of the grid fit is outside of the range of the graph, you should expand the range of the graph or "
                         "disregard this fit");
            PrintWarning("Optimal position for " + det.GetLabelName() + " was found to be at theta = " + std::to_string(minx) + " [" +
                         std::to_string(gr.GetXmin()) + ", " + std::to_string(gr.GetXmax()) + "]" + " phi = " + std::to_string(miy) + " [" +
                         std::to_string(gr.GetYmin()) + "," + std::to_string(gr.GetYmax()) + "] with value " + std::to_string(val));
        }
        else
        {
            PrintInfo_important("Optimal position for " + det.GetLabelName() + " is found for dTheta = " + std::to_string(minx) +
                                " dPhi = " + std::to_string(miy) + " with minimum value " + std::to_string(val));
        }
        
        det.SetOptimal_position(minx * TMath::DegToRad(), miy * TMath::DegToRad(), fitptr->IsValid());
    }
}

void OptiSpider::SpawnSegments()
{
    UserConf &confu = ConfManager::getInstance().user;

    fSegments.clear();
    int ndetectors =
        std::accumulate(fLUTProp->begin(), fLUTProp->end(), 0,
                        [&](int sum, const std::pair<unsigned int, std::unordered_map<unsigned int, SpiderLut::Properties>> &board) {
                            return sum + board.second.size();
                        });
    fSegments.reserve(ndetectors);
    for (auto it1 = fLUTProp->begin(); it1 != fLUTProp->end(); it1++)
    {
        auto _board = it1->first;
        auto _map   = it1->second;
        for (auto it2 = _map.begin(); it2 != _map.end(); it2++)
        {
            auto _channel = it2->first;
            auto _pos     = it2->second.pos;
            fSegments.emplace_back(it2->second.name, _board, _channel, _pos, fResultsFile);
            // fSegments.back().SetLabelName(it2->second.name);
            assert(confu.fit_range_start < confu.fit_range_end && "fit range start must be smaller than fit range end");
            fSegments.back().SetPeak(confu.fit_peak_energy, confu.fit_range_start, confu.fit_range_end);
        }
    }
}

void OptiSpider::ManipulateLUT(const TPositionOffset &offset)
{
    this->ReloadLUT();
    for (auto it1 = fLUTProp->begin(); it1 != fLUTProp->end(); it1++)
    {
        auto &_map = it1->second;
        for (auto it2 = _map.begin(); it2 != _map.end(); it2++)
        {
            auto &_element = it2->second;
            offset.OffsetVector(_element.pos);
        }
    }
}

void OptiSpider::OptimizeLUT()
{
    this->ReloadLUT();
    // iterate over segments
    int board, channel;
    for (auto &det : fSegments)
    {
        // find proper entry in LUT
        det.GetBoardChannel(board, channel);
        auto it1 = fLUTProp->find(board);
        if (it1 == fLUTProp->end()) std::runtime_error("Could not find board " + std::to_string(board) + " in SPIDER LUT");
        auto it2 = it1->second.find(channel);
        if (it2 == it1->second.end()) std::runtime_error("Could not find channel " + std::to_string(channel) + " in SPIDER LUT");
        auto &element = it2->second;

        element.pos= det.GetOptimalPosition();
    }
}

} // namespace gridopti
