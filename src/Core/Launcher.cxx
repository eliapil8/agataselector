#include "Launcher.h"
#include "ConfManager.h"
#include "DetectorAnalysis/DetectorAnalysis.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <utility>

template <class J>
int Launcher::Assignments<J>::nextJobId{1};

Launcher::Launcher() : settings(Settings::getInstance()), conf(ConfManager::getInstance()) {}

std::pair<int, Worker::Jobs> Launcher::getJob(const unsigned int &id)
{
    return getGeneralJob<Worker::Jobs>(id, jobs, jobMtx);
}

std::pair<int, std::vector<Worker::Jobs>> Launcher::getSumJobs(const unsigned int &id)
{
    return getGeneralJob<std::vector<Worker::Jobs>>(id, jobsToAdd, jobMtx);
}

void Launcher::Setup()
{
    if (settings.verbosity_level > 0)
        std::cout << "Setting up launcher" << std::endl;

    // Remove StopFile if present
    remove("StopFile");

    // check if conf file exist
    std::ifstream fileCheck(settings.confFileName, std::ifstream::in);
    fileCheck.close();

    if (!fileCheck.good())
        throw std::runtime_error("LAUNCHER Could not find conf file : " + settings.confFileName +
                                 "\n");
    fileCheck.close();

    // check if single file mode
    if (!settings.inFileName.empty() && !settings.outFileName.empty())
    {
        for (unsigned int i{0}; i < settings.inFileName.size(); ++i)
        {
            jobs.emplace_back(Worker::Jobs{-1, settings.inFileName.at(i), "",
                                           settings.outFileName.at(i), "", 0,
                                           DetectorAnalysis::Inputs::Type::SIGNAL});
            if (settings.subtract_bkg)
            {
                jobs.emplace_back(Worker::Jobs{-1, settings.inFileName.at(i), "",
                                               "lbkg_" + settings.outFileName.at(i), "", 0,
                                               DetectorAnalysis::Inputs::Type::BKG_LEFT});
                jobs.emplace_back(Worker::Jobs{-1, settings.inFileName.at(i), "",
                                               "rbkg_" + settings.outFileName.at(i), "", 0,
                                               DetectorAnalysis::Inputs::Type::BKG_RIGHT});
            }
        }
    }
    // if run numbers were specified
    else
    {
        for (const auto &runID : settings.runNr)
        {
            std::vector<Worker::Jobs> toAddInThisRun;

            std::stringstream RunID;
            RunID << std::setfill('0') << std::setw(4) << runID;

            std::string inFilePath = conf.replay.in_path + "/" + conf.replay.replay_dir_pattern +
                                     RunID.str() + conf.replay.in_sub_path + "/";

            std::vector<std::string> const fileName;

            DIR               *dir{nullptr};
            unsigned long long firstTS{0};
            if ((dir = opendir(inFilePath.c_str())) != nullptr)
            {
                std::vector<std::string> fileList;
                struct dirent           *ent;
                // Get all files in the requested directory
                while ((ent = readdir(dir)) != nullptr)
                {
                    fileList.emplace_back(ent->d_name);
                    // printf ("%s\n", ent->d_name);
                }
                std::sort(fileList.begin(), fileList.end());
                for (size_t fileNr{0}; fileNr < fileList.size(); ++fileNr)
                {
                    std::string const &tmpName = fileList[fileNr];
                    if (settings.verbosity_level > 0)
                        std::cout << "Found one generic file " << tmpName << std::endl;

                    // Check if the file pattern matches the required one
                    if (tmpName.compare(0, conf.replay.in_file_pattern.size(),
                                        conf.replay.in_file_pattern) == 0 &&
                        tmpName.substr(tmpName.find_last_of('.') + 1) == "root")
                    {
                        if (settings.verbosity_level > 0)
                            std::cout << "Found one input file " << tmpName << std::endl;
                        std::string const fileID =
                            tmpName.substr(conf.replay.in_file_pattern.size(), 4);
                        std::string const outFileName =
                            conf.replay.out_file_pattern + RunID.str() + "_" + fileID + ".root";

                        // Test if this file is ok and not unclosed
                        std::string const inFileNameTotal =
                            (inFilePath.empty() || inFilePath[inFilePath.length() - 1] == '/')
                                ? inFilePath + tmpName
                                : inFilePath + "/" + tmpName;

                        // If the file is ok and finished, add it to jobs to be
                        // performed
                        if (TFile testFile(inFileNameTotal.c_str(), "read");
                            testFile.IsOpen() &&
                            !testFile.IsZombie() /*!testFile.TestBit(TFile::kRecovered)*/)
                        {
                            if (tmpName.find("0000") != std::string::npos)
                            { // Find the first
                              // timestamp
                                if (settings.verbosity_level > 0)
                                    std::cout << "File is opened " << tmpName << std::endl;
                                auto *testTree = dynamic_cast<TTree *>(
                                    testFile.Get(conf.replay.tree_name.c_str()));
                                if (testTree == nullptr)
                                    throw std::runtime_error("Unable to get TTree in first file "
                                                             "which is needed for the timestamp\n");
                                firstTS = FindFirstTS(testTree);
                                if (settings.verbosity_level > 0)
                                    std::cout << "First TS is " << firstTS << std::endl;
                            }

                            // Check if the file has been already sorted if the
                            // --update_runs condition is set
                            const std::string outFileNameTotal =
                                (conf.replay.out_path.empty() ||
                                 conf.replay.out_path[conf.replay.out_path.length() - 1] == '/')
                                    ? conf.replay.out_path + outFileName
                                    : conf.replay.out_path + "/" + outFileName;

                            // IMPORTANT: with this method the update_runs
                            // option does not work properly if only one tree
                            // file as input
                            if ((settings.update_runs && !FileExists(outFileNameTotal)) ||
                                !settings.update_runs)
                            {
                                jobs.emplace_back(
                                    Worker::Jobs{static_cast<int>(runID), tmpName, inFilePath,
                                                 outFileName, conf.replay.out_path, firstTS,
                                                 DetectorAnalysis::Inputs::Type::SIGNAL});
                                if (settings.subtract_bkg)
                                {
                                    jobs.emplace_back(Worker::Jobs{
                                        static_cast<int>(runID), tmpName, inFilePath,
                                        "lbkg_" + outFileName, conf.replay.out_path, firstTS,
                                        DetectorAnalysis::Inputs::Type::BKG_LEFT});
                                    jobs.emplace_back(Worker::Jobs{
                                        static_cast<int>(runID), tmpName, inFilePath,
                                        "rbkg_" + outFileName, conf.replay.out_path, firstTS,
                                        DetectorAnalysis::Inputs::Type::BKG_RIGHT});
                                } // TODO: check if the bkg runs are there if
                                  // subtract_bkg is running
                            }

                            // If the file has already been analized, we still
                            // need to sum it with the others
                            toAddInThisRun.emplace_back(static_cast<int>(runID), outFileName,
                                                        conf.replay.out_path, "",
                                                        conf.replay.out_path, firstTS,
                                                        DetectorAnalysis::Inputs::Type::SIGNAL);
                            if (settings.subtract_bkg)
                            {
                                toAddInThisRun.emplace_back(
                                    static_cast<int>(runID), "lbkg_" + outFileName,
                                    conf.replay.out_path, "", conf.replay.out_path, firstTS,
                                    DetectorAnalysis::Inputs::Type::BKG_LEFT);
                                toAddInThisRun.emplace_back(
                                    static_cast<int>(runID), "rbkg_" + outFileName,
                                    conf.replay.out_path, "", conf.replay.out_path, firstTS,
                                    DetectorAnalysis::Inputs::Type::BKG_RIGHT);
                            }
                        }
                        else
                        {
                            std::cout << "file " << inFileNameTotal << " is not a good file!\n";
                        }
                    }
                }
                closedir(dir);
            }
            else
            {
                char cwd[PATH_MAX];
                auto i = getcwd(cwd, sizeof(cwd));
                (void)i;
                throw std::runtime_error("Could not open directory :" + inFilePath +
                                         " in directory " + cwd);
            }
            if (!settings.no_hadd && !toAddInThisRun.empty())
            {
                for (auto &j : toAddInThisRun)
                {
                    j.outFileName = conf.replay.sum_file_pattern + "-" +
                                    std::to_string(toAddInThisRun.front().runID) + "_" +
                                    std::to_string(toAddInThisRun.size()) + ".root";
                }

                listOfFinalFiles.emplace_back(conf.replay.out_path + "/" +
                                              toAddInThisRun.front().outFileName);
                jobsToAdd.emplace_back(toAddInThisRun);
            }
            else if (!jobs.empty())
            {
                for (auto &j : jobs)
                {
                    listOfFinalFiles.emplace_back(conf.replay.out_path + "/" + j.job.outFileName);
                }
            }
        }
    }

    if (!settings.outFileName.empty())
    {
        for (auto &j : settings.outFileName)
        {
            listOfFinalFiles.emplace_back(j);
        }
    }

    if (jobs.empty())
        throw std::runtime_error("No jobs to do!\n");
    if (settings.sum_all)
    {
        std::string outFileName = conf.replay.sum_all_pattern;
        for (const auto &j : jobsToAdd)
        {
            outFileName += "-";
            outFileName += std::to_string(j.job.front().runID);
        }
        outFileName += ".root";
        for (const auto &j : jobsToAdd)
        {
            sumAll.emplace_back(j.job.front().runID, j.job.front().outFileName,
                                conf.replay.out_path, outFileName, conf.replay.out_path, 0,
                                DetectorAnalysis::Inputs::Type::SIGNAL);
        }
    }

    // Create out folder if not present
    struct stat st{};
    if (stat(jobs.front().job.outPath.c_str(), &st) == -1)
    {
        mkdir(jobs.front().job.outPath.c_str(), 0777);
    }
}

void Launcher::Manager() {}

std::vector<std::string> Launcher::GetListOfFinalFiles() const
{
    return listOfFinalFiles;
}

std::string Launcher::GetSumAllFileName() const
{
    if (sumAll.empty())
        return {};
    const auto &job = sumAll.front();
    return (job.outPath.empty() || job.outPath.back() == '/') ? job.outPath + job.outFileName
                                                              : job.outPath + "/" + job.outFileName;
}

void Launcher::SumManager()
{
    // This should return directly as it is only useful for the distributed
    // version of the class
}

size_t Launcher::JobsInStack()
{
    return jobs.size();
}

void Launcher::finalizeJob(const unsigned int         &id,
                           const int                  &jobUniqueId,
                           const Worker::Jobs::Status &status)
{
    finalizeGeneralJob<Worker::Jobs>(id, jobUniqueId, status, jobs, jobMtx);
}

void Launcher::finalizeSumJobs(const unsigned int         &id,
                               const int                  &jobUniqueId,
                               const Worker::Jobs::Status &status)
{
    finalizeGeneralJob<std::vector<Worker::Jobs>>(id, jobUniqueId, status, jobsToAdd, jobMtx);
}

std::map<int, std::map<int, double>> Launcher::getProgress()
{
    std::map<int, std::map<int, double>> currentMap;
    for (const auto &n : progress)
        for (const auto &w : n.second)
            currentMap[n.first].emplace(w.first, w.second->load());

    return currentMap;
}

void Launcher::SumAll() const
{
    std::string command{"hadd -j 4 -f "};
    command += sumAll.front().outPath;
    command += "/";
    command += sumAll.front().outFileName;

    for (const auto &j : sumAll)
    {
        command += " ";
        command += j.inPath;
        command += "/";
        command += j.inFileName;
    }
    if (system(command.c_str()) != 0)
    {
        throw std::runtime_error("Failed to execute syscall: " + command + "\n");
    }
}

unsigned long long Launcher::FindFirstTS(TTree *testTree)
{
    std::map<std::string, std::string> detectors;
    detectors["AGATA"]    = "coreTS";
    detectors["PRISMA"]   = "TSPrisma";
    detectors["SPIDER"]   = "TSSPIDER";
    detectors["MONITOR"]  = "TSMONITOR";
    detectors["LABR"]     = "TSLABR";
    detectors["EUCLIDES"] = "TSEUCLIDES";
    detectors["DANTE"]    = "TSDANTE";
    detectors["MCP"]      = "TSMCP";
    detectors["EXSSIDE"]  = "TSEXSSIDE";
    detectors["OSCAR"]    = "TSOSCAR";
    detectors["SAURON"]   = "TSSAURON";

    std::map<std::string, unsigned long long> TStamps;
    for (const auto &det : detectors)
    {
        if (testTree->FindLeaf(det.second.c_str()) != nullptr)
        {
            testTree->SetBranchAddress(det.second.c_str(), &TStamps[det.first]);
        }
    }
    unsigned long long firstTS{0};
    long long          entry{0};
    const auto         entries{testTree->GetEntries()};
    while (firstTS == 0 && entry < entries)
    {
        testTree->GetEntry(entry++);
        for (const auto &ts : TStamps)
        {
            if (ts.second != 0)
            {
                firstTS = ts.second;
                break;
            }
        }
    }
    return firstTS;
}
