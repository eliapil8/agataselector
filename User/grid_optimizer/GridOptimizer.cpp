#include <algorithm>
#include <assert.h>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <map>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

#include <TSystem.h>
#include <TDatime.h>
#include <TFile.h>
#include <TH2D.h>
#include <TROOT.h>
#include <TStopwatch.h>
#include <TTree.h>


#include <filesystem>

//#include<fstream>
#include "App.h"
#include "ConfManager.h"
#include "Settings.h"
#include "SpiderLut.h"

#include "misc.hh"

void readListFile(const std::string &, std::vector<std::string> &, std::vector<std::string> &);

const std::string OPTIMIZED_FILE_PREFIX = "opti_";
TDatime           DATE_TIME;

#include "Conf_and_DummySelector/UserConf.h"
#include "OptiSpider.hpp"
#include "ROOTFileMerger.hpp"
#include "ROOTFileReducer.hpp"
#include "TPositionOffset.hpp"
#include "calibrator_misc.hpp"

void interrupt_handler(int s)
{
    PrintError("Interrupted by user, code " + std::to_string(s));
    exit(1);
}

void setup_interrupt()
{
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = interrupt_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);
}

std::string get_current_time()
{
    DATE_TIME.Set();
    std::string retval = "";
    auto        hour   = DATE_TIME.GetHour();
    auto        minute = DATE_TIME.GetMinute();
    auto        second = DATE_TIME.GetSecond();

    retval += (hour < 10 ? "0" : "") + std::to_string(hour) + ":";
    retval += (minute < 10 ? "0" : "") + std::to_string(minute) + ":";
    retval += (second < 10 ? "0" : "") + std::to_string(second);
    return retval;
}

void force_selector_settings()
{
    ConfManager &conf     = ConfManager::getInstance();
    Settings &   settings = Settings::getInstance();
    std::cout << "\n \n";
    PrintWarning("Be aware that some selector options are pre-set in the code:");
    PrintWarning_continue("       --sum_all             = true");
    PrintWarning_continue("       --no_hadd             = false");
    PrintWarning_continue("       --no_user_sel         = false");
    PrintWarning_continue("       --remove_partial      = true");
    PrintWarning_continue("       --only_enabled_histos = true");
    PrintWarning_continue("       --optimize            = false");
    PrintWarning_continue("       --verb                >= 2");
    // PrintWarning_continue("       --nrthr               = 6");
    PrintWarning_continue("       --update_runs         = false");
    PrintWarning_continue("       --reduction_condition = \"" + conf.user.data_reduction_condition + "\"*");
    PrintWarning_continue("        *data reduction is done internally, not on selector level ");
    PrintWarning_continue("         this achieves faster execution with same result ");

    settings.sum_all             = true;
    settings.no_hadd             = false;
    settings.userSelector        = true;
    settings.remove_partial      = true;
    settings.only_enabled_histos = true;
    settings.optimize            = false;
    settings.update_runs         = false;
    settings.reduction_condition = "";
    if(settings.verbosity_level < 2) settings.verbosity_level = 2;
    // settings.nrThr               = 6;

    std::cout << "\n \n";
    PrintWarning("Be aware that some configuration settings are pre-set the code if left unspecified:");
    PrintWarning_continue("     REPLAY_CONF-ENABLED_HISTOS is switched on if left unspecified");
    PrintWarning_continue("          list of enabled histograms is taken from GRID_SEARCH_OPTIMIZATION-ENABLED_HISTOS");
    PrintWarning_continue("          if left empty, the list contains only GRID_SEARCH_OPTIMIZATION-MATRIX_NAME");
    PrintWarning_continue("     AGATA_CONF-ENABLE_TREE  = NO");
    PrintWarning_continue("     AGATA_CONF-ENABLE_HISTS = YES");

    conf.agata.enable_histos   = true;
    conf.agata.enable_treeFill = false;

    std::cout << "\n \n";
}

std::vector<std::string> get_run_file_list(const int run_no)
{
    ConfManager &conf = ConfManager::getInstance();

    std::stringstream RunID;
    RunID << std::setfill('0') << std::setw(4) << run_no;

    std::string    inFilePath = conf.replay.in_path + "/" + conf.replay.replay_dir_pattern + RunID.str() + conf.replay.in_sub_path + "/";
    DIR *          dir{nullptr};
    struct dirent *ent;
    std::vector<std::string> fileList;
    std::vector<std::string> okfileList;
    if ((dir = opendir(inFilePath.c_str())) != nullptr)
    {
        // Get all files in the requested directory
        while ((ent = readdir(dir)) != nullptr) { fileList.emplace_back(ent->d_name); }
        std::sort(fileList.begin(), fileList.end());

        for (size_t fileNr{0}; fileNr < fileList.size(); ++fileNr)
        {
            std::string const tmpName = fileList[fileNr];

            // Check if the file pattern matches the required one
            if (tmpName.compare(0, conf.replay.in_file_pattern.size(), conf.replay.in_file_pattern) == 0 &&
                tmpName.substr(tmpName.find_last_of('.') + 1) == "root")
            {
                std::string const fileID      = tmpName.substr(conf.replay.in_file_pattern.size(), 4);
                std::string const outFileName = conf.replay.out_file_pattern + RunID.str() + "_" + fileID + ".root";

                // Test if this file is ok and not unclosed
                std::string const inFileNameTotal =
                    (inFilePath.empty() || inFilePath[inFilePath.length() - 1] == '/') ? inFilePath + tmpName : inFilePath + "/" + tmpName;

                TFile testFile(inFileNameTotal.c_str(), "READ");

                if (testFile.IsOpen() && !testFile.IsZombie()) { okfileList.emplace_back(inFileNameTotal); }
            }
        }
        closedir(dir);
    }
    return okfileList;
}

bool fileExists(const std::string &filename) { return std::filesystem::exists(filename); }

void merge_files()
{
    Settings &   settings = Settings::getInstance();
    ConfManager &conf     = ConfManager::getInstance();

    reducer::ROOTFileMerger merger(settings.nrThr);

    std::string merged_file_pattern = "merged_" + conf.replay.in_file_pattern;

    for (auto run : settings.runNr)
    {
        std::stringstream RunID;
        RunID << std::setfill('0') << std::setw(4) << run;
        std::string unmerged_in_path =
            conf.replay.in_path + '/' + conf.replay.replay_dir_pattern + RunID.str() + "/" + conf.replay.in_sub_path + '/';
        gridopti::RemoveDoubleSlashes(unmerged_in_path);

        std::string merged_path = unmerged_in_path + "merged/";
        gridopti::RemoveDoubleSlashes(merged_path);
        gridopti::createDirectories(merged_path);

        std::string merged_file = merged_path + merged_file_pattern + RunID.str() + ".root";
        if (fileExists(merged_file))
        {
            // std::cout << "Merged file " << merged_file << " already exists, skipping" << std::endl;
            continue;
        }
        // std::cout << "calling merger on " << unmerged_in_path << " " << merged_file << std::endl;
        merger.AddAllInDir(unmerged_in_path, merged_file);
    }
    merger.Merge();

    // conf.replay.in_path         = new_in_path;
    conf.replay.in_sub_path     = conf.replay.in_sub_path + "/merged/";
    conf.replay.in_file_pattern = merged_file_pattern;

    if (settings.verbosity_level > 1)
    {
        std::cout << "New conf.replay.in_sub_path       = " << conf.replay.in_sub_path << std::endl;
        std::cout << "New conf.replay.in_file_pattern   = " << conf.replay.in_file_pattern << std::endl;
    }
}

void reduce_files(const std::string &reduction_condition)
{
    Settings &   settings = Settings::getInstance();
    ConfManager &conf     = ConfManager::getInstance();

    // ##############################################################################

    auto reduction_manager = reducer::ROOTFileReducer();

    // unused branches
    // TODO add other detectors
    std::vector<std::string> unused_branches;
    if (!conf.detectorsPresent.Labr)
    {
        unused_branches.emplace_back("LABR*");
        unused_branches.emplace_back("TSLABR*");
        unused_branches.emplace_back("nbLABR");
    }
    if (!conf.detectorsPresent.Spider)
    {
        unused_branches.emplace_back("SPIDER*");
        unused_branches.emplace_back("TSSPIDER*");
        unused_branches.emplace_back("nbSPIDER");
    }

    // new file pattern must contain hash of the reduction condition since the file might have been already created by different reduction
    // condition
    size_t            condition_hash = std::hash<std::string>{}(reduction_condition);
    const std::string new_in_file_pattern{"reduced_" + std::to_string(condition_hash) + "_"};
    std::string       new_in_path = conf.user.reduced_file_path + '/' + "reduced_files";

    for (auto run : settings.runNr)
    {
        std::stringstream RunID;
        RunID << std::setfill('0') << std::setw(4) << run;

        std::string run_path = new_in_path + "/" + conf.replay.replay_dir_pattern + RunID.str() + "/" + conf.replay.in_sub_path + "/";
        gridopti::RemoveDoubleSlashes(run_path);
        gridopti::createDirectories(run_path);

        auto in_file_list = get_run_file_list(run);
        for (auto &infile : in_file_list)
        {
            std::string original_rootfilename = infile.substr(infile.find_last_of('/') + 1);
            std::string reduced_file_name     = run_path + new_in_file_pattern + original_rootfilename;
            if (fileExists(reduced_file_name)) continue;

            gridopti::RemoveDoubleSlashes(infile);
            gridopti::RemoveDoubleSlashes(reduced_file_name);

            reducer::ReductionJob job(infile, reduced_file_name, reduction_condition, conf.replay.tree_name, unused_branches);
            reduction_manager.AddReductionJob(std::move(job));
        }
    }
    reduction_manager.Reduce();

    // modify conf file
    conf.replay.in_path         = new_in_path;
    conf.replay.in_file_pattern = new_in_file_pattern + conf.replay.in_file_pattern;

    if (settings.verbosity_level > 1)
    {
        std::cout << "New conf.replay.in_path         = " << conf.replay.in_path << std::endl;
        std::cout << "New conf.replay.in_file_pattern = " << conf.replay.in_file_pattern << std::endl;
    }
}

std::string find_file(const std::string &path, const std::string &partialName, const std::string &suffix)
{
    for (const auto &entry : std::filesystem::directory_iterator(path))
    {
        std::string filename = entry.path().filename().string();
        if (filename.find(partialName) != std::string::npos && filename.find(suffix) != std::string::npos) { return entry.path().string(); }
    }
    return "";
}

std::vector<std::string> find_all_files(const std::string &path, const std::string &partialName, const std::string &suffix)
{
    std::vector<std::string> retval;
    for (const auto &entry : std::filesystem::directory_iterator(path))
    {
        std::string filename = entry.path().filename().string();
        if (filename.find(partialName) != std::string::npos && filename.find(suffix) != std::string::npos)
        { retval.emplace_back(entry.path().string()); }
    }
    return retval;
}

std::string format_duration(double seconds)
{
    int hours = static_cast<int>(seconds) / 3600;
    seconds -= hours * 3600;
    int minutes = static_cast<int>(seconds) / 60;
    seconds -= minutes * 60;

    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << hours << ":" << std::setw(2) << std::setfill('0') << minutes << ":" << std::setw(2)
       << std::setfill('0') << static_cast<int>(seconds);

    return ss.str();
}

/// @brief iterate through all the position combinations f
/// @param detectors
void run_selector_loop(std::vector<gridopti::OptiDetector *> &detectors)
{
    if (detectors.size() == 0) { std::runtime_error("No detector to optimize was declared"); }

    ConfManager &conf = ConfManager::getInstance();

    if (conf.user.run_preprocessing)
    {
        reduce_files(conf.user.data_reduction_condition);
        merge_files();
    }

    const std::vector<gridopti::TPositionOffset> offsets = (detectors.at(0)->GetOffsets());

    // remove already finish offsets
    std::vector<gridopti::TPositionOffset> not_run_offsets;
    for (const auto &off_pos : offsets)
    {
        std::string new_root_file_name =
            find_file(conf.replay.out_path, OPTIMIZED_FILE_PREFIX + off_pos.GenerateString() + "_run", ".root");
        if (new_root_file_name.empty()) not_run_offsets.emplace_back(off_pos);
    }

    int    task         = 0;
    double time_elapsed = 0;
    for (const auto &off_pos : not_run_offsets)
    {
        TStopwatch timer;
        timer.Start();
        std::cout << "\n";
        PrintInfo_important(get_current_time() + Form("  Working on task %i/%li, skipping %i tasks finished prior", ++task,
                                                      not_run_offsets.size(), (int)offsets.size() - (int)not_run_offsets.size()));
        // manipulate LUT values
        std::for_each(detectors.begin(), detectors.end(), [&off_pos](auto *det) { det->ManipulateLUT(off_pos); });

        conf.replay.sum_file_pattern = OPTIMIZED_FILE_PREFIX + off_pos.GenerateString() + "_run";

        {
            // redirect cout of selector to a file
            std::streambuf *oldCoutBuffer = std::cout.rdbuf();
            std::ofstream   file;
            if (task == 1) file = std::ofstream("selector_log.txt"); // Create new file
            else
                file = std::ofstream("selector_log.txt", std::ios_base::app); // just append to existing file
            std::cout.rdbuf(file.rdbuf());
            std::cout << "\n\n"
                      << " working on selector task " << task << " of " << not_run_offsets.size() << "\n\n";

            std::cout << "renaming sum file pattern to " << conf.replay.sum_file_pattern << std::endl;

            std::string new_root_file_name = find_file(conf.replay.out_path, conf.replay.sum_file_pattern, ".root");
            if (!new_root_file_name.empty())
            {
                std::cout.rdbuf(oldCoutBuffer);
                PrintInfo_normal("          File " + new_root_file_name + " already exists, skipping");
                continue;
            }
            // Now, all stdout's and stderr output will be written to output.txt
            gSystem->RedirectOutput("selector_root_log.txt", "a");
            App app;
            gSystem->RedirectOutput(0);
            // reestablish interrupt handler
            setup_interrupt();
            // When done, restore the old std::cout buffer
            std::cout.rdbuf(oldCoutBuffer);
            file.close();
        }

        std::vector<std::string> just_produced_root_files = find_all_files(conf.replay.out_path, conf.replay.sum_file_pattern, ".root");
        if (just_produced_root_files.empty()) throw std::runtime_error("Could not find just produced root file");

        for (const auto &filename : just_produced_root_files)
        {
            TFile *file = TFile::Open(filename.c_str(), "UPDATE");
            if (!file || file->IsZombie()) { throw std::runtime_error("Could not open just produced root file"); }

            file->cd();
            off_pos.Write("position_offset");
            file->Close();
        }
        auto time = timer.RealTime();
        time_elapsed += time;
        auto time_estimate = time_elapsed / static_cast<double>(task) * (static_cast<double>(not_run_offsets.size()) - static_cast<double>(task));
        std::cout << "Task " << task << " finished in ";
        timer.Print();
        PrintInfo_important("Estimated time remaining for selector loop stage: " + format_duration(time_estimate));
    }
}

void process_optimizer(std::vector<gridopti::OptiDetector *> &detectors)
{
    PrintInfo_important(get_current_time() + Form("  Starting to produce optimized files"));

    PrintInfo_normal("The fitter is set up with following parameters: ");
    PrintInfo_normal("   peak_fit_function       = " + ConfManager::getInstance().user.peak_fit_function);
    PrintInfo_normal("   peak_fit_strategy       = " + ConfManager::getInstance().user.peak_fit_width_strategy);
    PrintInfo_normal("   peak_fit_strategy_value = " + std::to_string(ConfManager::getInstance().user.peak_fit_width_strategy_value));

    assert(!detectors.empty() && "No detector to optimize was declared");
    ConfManager &conf = ConfManager::getInstance();

    std::array<std::string, 3> rootfile_names;
    // make sure 0 offset (default) is produced by trying to run it
    {
        // preparation for selector
        PrintInfo_important(get_current_time() + Form("  Producing zero offset file for comparison"));
        gridopti::TPositionOffset zero_offset(0., 0.);
        std::for_each(detectors.begin(), detectors.end(), [&zero_offset](auto *det) { det->ManipulateLUT(zero_offset); });

        conf.replay.sum_file_pattern   = OPTIMIZED_FILE_PREFIX + zero_offset.GenerateString() + "_run";
        std::string new_root_file_name = find_file(conf.replay.out_path, conf.replay.sum_file_pattern, ".root");
        if (new_root_file_name.empty())
        {
            // redirect cout of selector
            std::streambuf *oldCoutBuffer = std::cout.rdbuf();
            std::ofstream   file          = std::ofstream("selector_log.txt", std::ios_base::app);
            std::cout.rdbuf(file.rdbuf());
            // run selector
            gSystem->RedirectOutput("selector_root_log.txt", "a");
            App app;
            gSystem->RedirectOutput(0);
            // reestablish interrupt handler
            setup_interrupt();
            // redirect cout back
            std::cout.rdbuf(oldCoutBuffer);
            file.close();
        }
        // add file to list of rootfiles for direct retrieval
        rootfile_names[0] = find_file(conf.replay.out_path, conf.replay.sum_file_pattern, ".root");
    }
    // now run with optimized parameters, allowing ONLY GOOD fits to be used
    {
        PrintInfo_important(get_current_time() + Form("  Producing optimized files - only good fits"));
        bool use_bad_fits = false;
        // calculate optimized positions
        std::for_each(detectors.begin(), detectors.end(), [&use_bad_fits](auto *det) { det->Optimize(use_bad_fits); });
        // save new LUT to file
        // std::for_each(detectors.begin(), detectors.end(), [&use_bad_fits](auto *det) { det->SaveOptimizedLUT(use_bad_fits); });
        // set LUT to optimal positions
        std::for_each(detectors.begin(), detectors.end(), [&use_bad_fits](auto *det) { det->OptimizeLUT(); });

        conf.replay.sum_file_pattern = OPTIMIZED_FILE_PREFIX + "optimizedPosition_good_fits_only_run";
        PrintInfo_important("          Running optimized selector using also bad fits, results saved to " + conf.replay.sum_file_pattern);
        {
            // redirect cout of selector
            std::streambuf *oldCoutBuffer = std::cout.rdbuf();
            std::ofstream   file          = std::ofstream("selector_log.txt", std::ios_base::app);
            std::cout.rdbuf(file.rdbuf());
            // run selector
            gSystem->RedirectOutput("selector_root_log.txt", "a");
            App app;
            gSystem->RedirectOutput(0);
            // reestablish interrupt handler
            setup_interrupt();
            // redirect cout back
            std::cout.rdbuf(oldCoutBuffer);
            file.close();
        }
        // add file to list of rootfiles for direct retrieval
        rootfile_names[1] = find_file(conf.replay.out_path, conf.replay.sum_file_pattern, ".root");
    }
    // now run with optimized parameters, allowing ALSO BAD fits to be used
    {
        PrintInfo_important(get_current_time() + Form("  Producing optimized files - also bad fits"));
        bool use_bad_fits = true;
        // calculate optimized positions
        std::for_each(detectors.begin(), detectors.end(), [&use_bad_fits](auto *det) { det->Optimize(use_bad_fits); });
        // save new LUT to file
        // std::for_each(detectors.begin(), detectors.end(), [&use_bad_fits](auto *det) { det->SaveOptimizedLUT(use_bad_fits); });
        // set LUT to optimal positions
        std::for_each(detectors.begin(), detectors.end(), [&use_bad_fits](auto *det) { det->OptimizeLUT(); });

        conf.replay.sum_file_pattern = OPTIMIZED_FILE_PREFIX + "optimizedPosition_bad_fits_also_run";
        PrintInfo_important("          Running optimized selector using only good, results saved to " + conf.replay.sum_file_pattern);

        {
            // redirect cout of selector
            std::streambuf *oldCoutBuffer = std::cout.rdbuf();
            std::ofstream   file          = std::ofstream("selector_log.txt", std::ios_base::app);
            std::cout.rdbuf(file.rdbuf());
            // run selector
            gSystem->RedirectOutput("selector_root_log.txt", "a");
            App app;
            gSystem->RedirectOutput(0);
            // reestablish interrupt handler
            setup_interrupt();
            // redirect cout back
            std::cout.rdbuf(oldCoutBuffer);
            file.close();
        }
        // add file to list of rootfiles for direct retrieval
        rootfile_names[2] = find_file(conf.replay.out_path, conf.replay.sum_file_pattern, ".root");
    }

    // compare histograms
    {
        std::cout << " ROOT FILE NAMES: " << rootfile_names[0] << std::endl;
        std::cout << " ROOT FILE NAMES: " << rootfile_names[1] << std::endl;
        std::cout << " ROOT FILE NAMES: " << rootfile_names[2] << std::endl;

        std::array<TFile, 3>  rfiles{TFile(rootfile_names[0].c_str(), "READ"), TFile(rootfile_names[1].c_str(), "READ"),
                                    TFile(rootfile_names[2].c_str(), "READ")};
        std::array<TH2D *, 3> matrices;
        matrices[0] = dynamic_cast<TH2D *>(gridopti::OptiDetector::FindHistoInROOTFile(&rfiles[0], conf.user.matrix_name.c_str()));
        matrices[1] = dynamic_cast<TH2D *>(gridopti::OptiDetector::FindHistoInROOTFile(&rfiles[1], conf.user.matrix_name.c_str()));
        matrices[2] = dynamic_cast<TH2D *>(gridopti::OptiDetector::FindHistoInROOTFile(&rfiles[2], conf.user.matrix_name.c_str()));
        assert(matrices[0] && matrices[1] && matrices[2] && "Could not get matrices from root files");

        //rename matrices
        std::string mat_name = "original_" + std::string(matrices[0]->GetName());
        matrices[0]->SetName(mat_name.c_str());
        mat_name = "opti_onlyGoodFits_" + std::string(matrices[1]->GetName());
        matrices[1]->SetName(mat_name.c_str());
        mat_name = "opti_alsoBadFits_" + std::string(matrices[2]->GetName());
        matrices[2]->SetName(mat_name.c_str());

        int   nxbins = matrices[0]->GetNbinsX();
        int   nybins = matrices[0]->GetNbinsY();
        TFile root_out("opti_compare.root", "RECREATE");

        // make total projections first
        {

            TH1D *orig_total      = matrices[0]->ProjectionY("orig_total", 1, nxbins);
            TH1D *opti_good_total = matrices[1]->ProjectionY("opti_good_total", 1, nxbins);
            TH1D *opti_bad_total  = matrices[2]->ProjectionY("opti_bad_total", 1, nxbins);
            assert(opti_bad_total && opti_good_total && orig_total && "Could get total projection histograms");

            std::string matrix_units    = gridopti::OptiDetector::GetMatrixUnit(matrices[0]);
            double      unit_correction = !matrix_units.empty() ? 1. / UNITS::classify(matrix_units) : 1.;

            double xmin = conf.user.fit_range_start * unit_correction;
            double xmax = conf.user.fit_range_end * unit_correction;

            orig_total->GetXaxis()->SetRangeUser(xmin, xmax);
            opti_good_total->GetXaxis()->SetRangeUser(xmin, xmax);
            opti_bad_total->GetXaxis()->SetRangeUser(xmin, xmax);
            double ymax = std::max({orig_total->GetMaximum(), opti_good_total->GetMaximum(), opti_bad_total->GetMaximum()});
            orig_total->GetYaxis()->SetRangeUser(0, ymax * 1.1);
            opti_good_total->GetYaxis()->SetRangeUser(0, ymax * 1.1);
            opti_bad_total->GetYaxis()->SetRangeUser(0, ymax * 1.1);

            TCanvas canvas("Total_projections", "Total projections", 800, 600);
            orig_total->SetLineColor(kBlue);
            opti_good_total->SetLineColor(kBlack);
            opti_bad_total->SetLineColor(kRed);

            orig_total->Draw();
            opti_good_total->Draw("same");
            opti_bad_total->Draw("same");
            root_out.cd();
            canvas.Write();
            orig_total->Write();
            opti_good_total->Write();
            opti_bad_total->Write();

            orig_total->SetDirectory(0);
            opti_good_total->SetDirectory(0);
            opti_bad_total->SetDirectory(0);

            delete orig_total;
            delete opti_good_total;
            delete opti_bad_total;
        }

        PrintInfo_normal(" Files with original and optimized histograms are saved in " + std::string(root_out.GetName()));
        for (int bin = 1; bin < nxbins; bin++)
        {
            std::string label_name = matrices[0]->GetXaxis()->GetBinLabel(bin);
            std::string label_orig = label_name + "_orig";
            std::string label_good = label_name + "_only_good_fit";
            std::string label_bad  = label_name + "_also_bad_fit";

            TH1D *orig = matrices[0]->ProjectionY(label_orig.c_str(), bin, bin);
            assert(orig && "Could not get original histogram");
            if (orig->Integral(1, nybins) == 0)
            {
                delete orig;
                continue;
            }
            TH1D *opti_good = matrices[1]->ProjectionY(label_good.c_str(), bin, bin);
            assert(opti_good && "Could not get opti_good histogram");
            TH1D *opti_bad = matrices[2]->ProjectionY(label_bad.c_str(), bin, bin);
            assert(opti_bad && "Could not get opti_bad histogram");

            TDirectory *dir = root_out.mkdir(label_name.c_str());
            dir->cd();
            TCanvas canvas(label_name.c_str(), label_name.c_str(), 800, 600);
            orig->SetLineColor(kBlue);
            opti_good->SetLineColor(kBlack);
            opti_bad->SetLineColor(kRed);

            std::string matrix_units    = gridopti::OptiDetector::GetMatrixUnit(matrices[0]);
            double      unit_correction = !matrix_units.empty() ? 1. / UNITS::classify(matrix_units) : 1.;

            double xmin = conf.user.fit_range_start * unit_correction;
            double xmax = conf.user.fit_range_end * unit_correction;

            orig->GetXaxis()->SetRangeUser(xmin, xmax);
            opti_good->GetXaxis()->SetRangeUser(xmin, xmax);
            opti_bad->GetXaxis()->SetRangeUser(xmin, xmax);
            double ymax = std::max({orig->GetMaximum(), opti_good->GetMaximum(), opti_bad->GetMaximum()});
            orig->GetYaxis()->SetRangeUser(0, ymax * 1.1);
            opti_good->GetYaxis()->SetRangeUser(0, ymax * 1.1);
            opti_bad->GetYaxis()->SetRangeUser(0, ymax * 1.1);

            orig->Draw();
            opti_good->Draw("same");
            opti_bad->Draw("same");
            dir->cd();
            canvas.Write();
            orig->Write();
            opti_good->Write();
            opti_bad->Write();

            orig->SetDirectory(0);
            opti_good->SetDirectory(0);
            opti_bad->SetDirectory(0);
            delete orig;
            delete opti_good;
            delete opti_bad;
        }
        root_out.cd();
        matrices[0]->Write();
        matrices[1]->Write();
        matrices[2]->Write();
        root_out.Close();

        matrices[0]->SetDirectory(0);
        matrices[1]->SetDirectory(0);
        matrices[2]->SetDirectory(0);
        delete matrices[0];
        delete matrices[1];
        delete matrices[2];
    }
}

/// @brief Produce list of root files produced by the selector
/// @return 
std::vector<std::string> get_selector_file_list()
{
    Settings &   settings = Settings::getInstance();
    ConfManager &conf     = ConfManager::getInstance();

    std::vector<std::string> retval;

    std::string run_suffix = "run";
    for (auto nr : settings.runNr) run_suffix += "-" + std::to_string(nr);
    run_suffix += ".root";

    for (const auto &entry : fs::directory_iterator(conf.replay.out_path))
    {
        std::string path = entry.path();
        // std::cout << path << std::endl;
        // std::cout << fs::is_regular_file(path) << " " << (path.find(OPTIMIZED_FILE_PREFIX) != std::string::npos) << " "
        //           << (path.find(run_suffix) != std::string::npos) << std::endl;
        if (fs::is_regular_file(path) && path.find(OPTIMIZED_FILE_PREFIX) != std::string::npos &&
            path.find(run_suffix) != std::string::npos && path.find("optimizedPosition") == std::string::npos)
            retval.emplace_back(path);
    }

    

    if (settings.verbosity_level > 1)
    {
        std::cout << "Found " << retval.size() << " selector files" << std::endl;
        for (auto &x : retval) std::cout << "            " << x << std::endl;
    }

    return retval;
}

void process_selector_configuration(int argc, char *argv[])
{
    Settings &   settings = Settings::getInstance();
    ConfManager &conf     = ConfManager::getInstance();

    // Initialize settings and configuration singletons
    settings.parseOptions(argc, argv);

    std::cout << "***************************************************************" << std::endl;
    // If no args simply print help
    if (argc == 1 || settings.printHelp)
    {
        settings << std::cout;
        exit(0);
    }

    std::ifstream const confFile(settings.confFileName);
    if (!confFile.is_open())
    {
        int const val = symlink(("./Conf/" + settings.confFileName).c_str(), ("./" + settings.confFileName).c_str());
        if (val != 0) throw std::runtime_error("Unable to generate selector.conf symbolic link");
    }
    conf.ReadConf(settings.confFileName);

    force_selector_settings();

    // if conf.user.optiresult_file_path is left unspecified, the final root file will be saved in one of the reduced_file folders
    if (conf.user.optiresult_file_path.empty())
    {
        std::string exePath            = argv[0];
        conf.user.optiresult_file_path = exePath.substr(0, exePath.find_last_of("/\\"));
    }

    // fix list of enabled histos
    if (conf.user.enabled_histos.empty()) { conf.user.enabled_histos = conf.user.matrix_name; }
    // convert to vector of strings
    std::vector<std::string> enabled_histos_list;
    {
        std::string       str = conf.user.enabled_histos;
        std::stringstream ss(str);
        std::string       token;

        while (std::getline(ss, token, ' '))
        {
            if (!token.empty()) { enabled_histos_list.push_back(token); }
        }
    }

    // check if enabled histos file exists
    std::string enable_histo_path = conf.replay.conf_path;
    if (enable_histo_path.back() != '/') enable_histo_path += '/';
    if (conf.replay.enabled_histo_file_name.empty()) { conf.replay.enabled_histo_file_name = "enable_histos.txt"; }
    enable_histo_path += conf.replay.enabled_histo_file_name;

    std::vector<std::string> missingHistos;
    std::ifstream            inFile(enable_histo_path);
    if (inFile.good())
    {
        std::string line;
        for (const auto &hname : enabled_histos_list)
        {
            bool found = false;
            while (std::getline(inFile, line))
            {
                if (line.find(hname) != std::string::npos)
                {
                    found = true;
                    break;
                }
            }
            if (!found) { missingHistos.push_back(hname); }
            inFile.clear();  // Clear EOF flag
            inFile.seekg(0); // Go back to the start of the file
        }
    }
    else
    {
        // The file does not exist, so all histos are missing
        missingHistos = enabled_histos_list;
    }
    inFile.close();

    if (!missingHistos.empty())
    {
        std::ofstream outFile(conf.replay.enabled_histo_file_name, std::ios::app);
        for (const auto &histo : missingHistos) { outFile << histo << "\n"; }
        outFile.close();
    }
}

void process_ROOT_files(std::vector<gridopti::OptiDetector *> optimizedDetector, bool force_new_file)
{
    ConfManager &conf = ConfManager::getInstance();
    if (force_new_file)
    {
        // we have to remove old files to force production of new ones
        for (auto &det : optimizedDetector)
        { 
            std::string fname = find_file(conf.replay.out_path, OPTIMIZED_FILE_PREFIX + "processed_" + det->GetName(), ".root"); 
            // std::remove(fname.c_str());
        }
    }

    auto opti_filelist = get_selector_file_list();
    // remove non-standard opti_files -> ones that are produced at the end of optimization process
    std::remove_if(opti_filelist.begin(), opti_filelist.end(), [](std::string &filename) { return (filename.find("optimizedPosition") != std::string::npos); });
    

    assert(!opti_filelist.empty() && "No optimized files found");
    // assign bin numbers of a matrix of interest to each segment
    std::for_each(optimizedDetector.begin(), optimizedDetector.end(),
                  [&opti_filelist](auto *det) { det->SetBinNumbers(opti_filelist.front()); });

    // process only detector fits that were not processed before
    // std::vector<gridopti::OptiDetector *> detectors_to_process;
    // std::for_each(optimizedDetector.begin(), optimizedDetector.end(), [&](auto *det) {
    //     if (!det->AreFitsProcessed()) { detectors_to_process.emplace_back(det); }
    // });

    // if (detectors_to_process.empty())
    // {
    //     PrintInfo_important("All detectors were already processed, skipping process_ROOT_files step");
    //     return;
    // }

    

    double time_elapsed = 0;
    for (unsigned int task = 1; task <= opti_filelist.size(); ++task)
    {
        std::string filename{opti_filelist.at(task - 1)};

        TStopwatch timer;
        timer.Start();
        std::cout << std::endl;
        PrintInfo_important(get_current_time() + "   Working on OptiFile processing task " + std::to_string(task) + "/" +
                            std::to_string(opti_filelist.size()) + ", file: " + filename);

        std::for_each(optimizedDetector.begin(), optimizedDetector.end(),
                      [&](auto *det) { det->ProcessROOTFile(filename, conf.user.matrix_name); });

        auto time = timer.RealTime();
        time_elapsed += time;
        auto time_estimate =
            time_elapsed / static_cast<double>(task) * (static_cast<double>(opti_filelist.size()) - static_cast<double>(task));
        std::cout << get_current_time() << "       Task " << task << " finished in ";
        timer.Print();
        PrintInfo_important("       Estimated time remaining for optiFile processing: " + format_duration(time_estimate) + "\n");
    }
    // save data to file
    std::for_each(optimizedDetector.begin(), optimizedDetector.end(), [&](auto *det) { det->FlushDataToFile(); });
}

int main(int argc, char *argv[])
{   

    setup_interrupt();
    // to clear the log file
    std::remove("selector_root_log.txt");
    
    PrintInfo_important("Selector logs can be found in selector_log.txt (output of selector) and selector_root_log.txt (output produced by ROOT objects - mostly by hadd)");

    // set paths
    if(system("source Scripts/selector.sh") != 0 || system("source Scripts/selector.sh") != 0)
    {
        std::cerr << "Could not source selector.sh" << std::endl;
    }
 
    gROOT->SetBatch(kTRUE);

    process_selector_configuration(argc, argv);

    ConfManager &                         conf = ConfManager::getInstance();
    std::vector<gridopti::OptiDetector *> optimizedDetector;

    // create detectors
    if (conf.detectorsPresent.Spider) optimizedDetector.emplace_back(new gridopti::OptiSpider());
    if (conf.detectorsPresent.Euclides) PrintError("Euclides not implemented yet");
    if (conf.detectorsPresent.OscarUp) PrintError("Oscar up not implemented yet");
    if (conf.detectorsPresent.OscarDown) PrintError("Oscar down not implemented yet");
    if (conf.detectorsPresent.SauronBw || conf.detectorsPresent.SauronFw) PrintError("Sauron not implemented yet");
    if (conf.detectorsPresent.Dante) PrintError("Dante not implemented yet");
    assert(!optimizedDetector.empty() && "No detector to optimize was declared");

    // produce opti files = run selector
    if (conf.user.run_selector_loop) run_selector_loop(optimizedDetector);

    // enable multithreading, it helps with fitting which cannot be multithreaded due to ROOT limitations
    ROOT::EnableThreadSafety();
    ROOT::EnableImplicitMT();

    // process opti files
    process_ROOT_files(optimizedDetector, conf.user.run_root_file_processing);
    // optimize data

    process_optimizer(optimizedDetector);

    // now we should run the selector again with optimized LUTs

    // delete to save data to root file and free memory
    for (auto *det : optimizedDetector)
    {
        delete det;
        det = nullptr;
    }
    optimizedDetector.clear();

    return 0;
}
