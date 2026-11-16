#include "Worker.h"

#include <algorithm>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <sys/un.h>
#include <thread>

#include <ROOT/RDataFrame.hxx>
#include <ROOT/TProcessExecutor.hxx>
#include <TFile.h>
#include <TFileMerger.h>
#include <TStopwatch.h>
#include <TThread.h>
#include <TTree.h>

#include <atomic>

#include "AgataSelector.h"
#include "MemCheck.h"
#include "UserSelector.h"

void Worker::Sel()
{
    while (true)
    {
        auto [validJob, j] = getJob(id);
        // auto getJobReturn = getJob(id);
        // auto validJob{getJobReturn.first};
        // auto j{getJobReturn.second};

        // Check if StopFile is present
        {
            std::ifstream f("StopFile");
            if (f.good())
                return;
        }

        if (validJob < 0)
            break;
        progress.store(-2.);
        // std::cout << "[SLAVE] Received valid job: " << j.outFileName <<
        // std::endl;

        // prisma branches update
        if (settings.update_prisma)
        {
            std::stringstream RunID;
            RunID << std::setfill('0') << std::setw(4) << j.runID;

            std::string prismaFilePath = conf.prisma.prisma_in_path + "/" +
                                         conf.prisma.prisma_dir_pattern + RunID.str() + "/";

            std::string replayFilePath =
                (j.inPath.empty() || j.inPath[j.inPath.length() - 1] == '/') ? j.inPath
                                                                             : j.inPath + "/";
            std::string replayFileName = j.inFileName;

            updatePrisma(replayFilePath, replayFileName, prismaFilePath);
            continue;
        }

        std::string const inFileNameTotal =
            (j.inPath.empty() || j.inPath[j.inPath.length() - 1] == '/')
                ? j.inPath + j.inFileName
                : j.inPath + "/" + j.inFileName;
        std::string const outFileNameTotal =
            (j.outPath.empty() || j.outPath[j.outPath.length() - 1] == '/')
                ? j.outPath + j.outFileName
                : j.outPath + "/" + j.outFileName;

        TFile *file = nullptr;
        file        = new TFile(inFileNameTotal.c_str(), "read");

        if (!file->IsOpen())
            throw std::runtime_error("Could not find selected TFile\n");
        if (file->IsZombie())
            throw std::runtime_error("TFile not closed and cannot be recovered\n");
        // if(file->TestBit(TFile::kRecovered)) {}; // It's ok!

        auto *tree = dynamic_cast<TTree *>(file->Get(conf.replay.tree_name.c_str()));
        if (tree == nullptr)
            throw std::runtime_error("Could not find TTree in TFile\n");

        // tree reduction
        if (!settings.reduction_condition.empty())
        {
            std::string const outRedName =
                (j.inPath.empty() || j.inPath[j.inPath.length() - 1] == '/')
                    ? j.inPath + "red_" + j.inFileName
                    : j.inPath + "/red_" + j.inFileName;

            auto *redFile = new TFile(outRedName.c_str(), "recreate");
            if (!redFile->IsOpen())
                throw std::runtime_error("Could not create new TFile\n");
            if (redFile->IsZombie())
                throw std::runtime_error("new TFile IsZombie\n");

            redFile->cd();
            TTree *redTree = tree->CopyTree(settings.reduction_condition.c_str());

            redTree->Write();
            redFile->Write();
            redFile->Close();

            if (settings.verbosity_level > 1)
            {
                std::cout << "Deleting red tree" << std::endl;
            }
            delete redFile;
            continue;
        }
        TFile          outFile(outFileNameTotal.c_str(), "recreate");
        AgataSelector *selector;
        if (settings.userSelector)
        {
            selector = new UserSelector(outFile);
        }
        else
        {
            selector = new AgataSelector(outFile);
        }
        selector->ConfProgress(&progress);
        selector->SetFirstTS(j.initialTimestamp);
        selector->SetJobType(j.type);

        progress.store(-1.);
        try
        {
            if (settings.nrEvts < 0)
                tree->Process(selector, "");
            else
                tree->Process(selector, "", settings.nrEvts);
        }
        catch (const MemCheck::OutOfMemoryException &e)
        {
            std::cerr
                << "Using too many threads given the available memory. Trying to manage the threads"
                << std::endl;
            std::cerr << e.what() << std::endl;
            file->Close();
            delete file;
            delete selector;
            finalizeJob(id, validJob, Jobs::Status::MEMORYERROR); // Unable to finish job
            sleep(2);
            // continue;
            return;
        }
        if (settings.verbosity_level > 1)
        {
            std::cout << "Deleting selector" << std::endl;
        }
        delete selector;
        file->Close();
        if (settings.verbosity_level > 1)
        {
            std::cout << "File closed, deleting" << std::endl;
        }
        delete file;

        finalizeJob(id, validJob, Jobs::Status::FINISHED); // Finished job correctly
    }
}

void Worker::Sum()
{
    while (true)
    {
        auto [validJob, jbs] = getSumJob(id);

        // Check if StopFile is present
        {
            std::ifstream f("StopFile");
            if (f.good())
                return;
        }

        if (validJob < 0)
            break;

        std::string outFileName{(jbs.front().outPath.empty() ||
                                 jbs.front().outPath[jbs.front().outPath.length() - 1] == '/')
                                    ? jbs.front().outPath + jbs.front().outFileName
                                    : jbs.front().outPath + "/" + jbs.front().outFileName};

        std::cout << "Output file name of sum will be : " << outFileName << std::endl;
        std::vector<std::string> outFiles;
        for (const auto &j : jbs)
        {
            outFiles.push_back((j.inPath.empty() || j.inPath[j.inPath.length() - 1] == '/')
                                   ? j.inPath + j.inFileName
                                   : j.inPath + "/" + j.inFileName);
        }
        if (jbs.empty())
            continue;

        TStopwatch timer;
        timer.Start();

        if (jbs.size() == 1)
        {
            std::string cmd;
            cmd = "cp ";
            cmd += outFiles.at(0);
            cmd += " " + outFileName;
            if (system(cmd.c_str()) != 0)
                throw std::runtime_error("Move file returned a bad value\n");
        }
        else
        {
            ROOT::TIOFeatures  features;
            Bool_t             append                   = kFALSE;
            Bool_t             force                    = kTRUE;
            Bool_t             skip_errors              = kTRUE;
            Bool_t             reoptimize               = kFALSE;
            Bool_t             noTrees                  = kFALSE;
            Bool_t             keepCompressionAsIs      = kFALSE;
            Bool_t             useFirstInputCompression = kFALSE;
            Bool_t             multiproc                = kTRUE;
            Int_t              maxopenedfiles           = 0;
            Int_t              verbosity                = 99;
            TString            cacheSize;
            const unsigned int maxHaddThreads{10};
            auto               nProcesses = static_cast<float>(sqrt(jbs.size())) < maxHaddThreads
                                                ? static_cast<unsigned int>(sqrt(jbs.size()))
                                                : maxHaddThreads;
            Int_t              newcomp    = -1;

            if (verbosity > 5)
            {
                std::cout << "hadd Target file: " << outFileName << std::endl;
            }

            TFileMerger fileMerger(kFALSE, kFALSE);
            fileMerger.SetMsgPrefix("hadd");
            fileMerger.SetPrintLevel(verbosity - 1);
            if (maxopenedfiles > 0)
            {
                fileMerger.SetMaxOpenedFiles(maxopenedfiles);
            }
            if (newcomp == -1)
            {
                if (useFirstInputCompression || keepCompressionAsIs)
                {
                    // grab from the first file.
                    TFile        *firstInput = nullptr;
                    std::ifstream check_file(outFiles.front());
                    if (!check_file.is_open())
                    {
                        std::cerr << "hadd could not open indirect file " << std::endl;
                        throw std::runtime_error("hadd was unable to merge the file\n");
                    }
                    firstInput = TFile::Open(outFiles.front().c_str());
                    if (firstInput && !firstInput->IsZombie())
                        newcomp = firstInput->GetCompressionSettings();
                    else
                        newcomp = ROOT::RCompressionSetting::EDefaults::kUseCompiledDefault % 100;
                    if (settings.verbosity_level > 1)
                    {
                        std::cout << "Deleting " << outFiles.front() << std::endl;
                    }
                    delete firstInput;
                }
                else
                    newcomp = ROOT::RCompressionSetting::EDefaults::kUseCompiledDefault %
                              100; // default compression level.
            }
            if (settings.verbosity_level > 1)
            {
                if (keepCompressionAsIs && !reoptimize)
                    std::cout << "hadd compression setting for meta data: " << newcomp << '\n';
                else
                    std::cout << "hadd compression setting for all output: " << newcomp << '\n';
            }
            if (append)
            {
                if (!fileMerger.OutputFile(outFileName.c_str(), "UPDATE", newcomp))
                {
                    std::cerr << "hadd error opening target file for update " << std::endl;
                    throw std::runtime_error("hadd was unable to merge the file\n");
                }
            }
            else if (!fileMerger.OutputFile(outFileName.c_str(), force, newcomp))
            {
                std::cerr << "hadd error opening target file (does it exist?)." << std::endl;
                if (!force)
                    std::cerr << "Pass \"-f\" argument to force re-creation "
                                 "of output file."
                              << std::endl;
                throw std::runtime_error("hadd was unable to merge the file\n");
            }

            auto filesToProcess = outFiles.size();
            auto step           = (filesToProcess + nProcesses - 1) / nProcesses;
            if (multiproc && step < 3)
            {
                // At least 3 files per process
                step       = 3;
                nProcesses = (filesToProcess + step - 1) / step;
                std::cout << "Each process should handle at least 3 files for "
                             "efficiency.";
                std::cout << " Setting the number of processes to: " << nProcesses << std::endl;
            }
            if (nProcesses == 1)
                multiproc = kFALSE;

            std::vector<std::string> partialFiles;
            if (multiproc)
            {
                auto uuid        = TUUID();
                auto partialTail = uuid.AsString();
                for (auto i = 0; (i * step) < filesToProcess; i++)
                {
                    std::stringstream buffer;
                    buffer << conf.replay.out_path << "/" << i << "_" << partialTail << ".root";
                    partialFiles.emplace_back(buffer.str());
                }
            }

            auto mergeFiles = [&](TFileMerger &merger) {
                if (reoptimize)
                {
                    merger.SetFastMethod(kFALSE);
                }
                else
                {
                    if (!keepCompressionAsIs && merger.HasCompressionChange())
                    {
                        // Don't warn if the user any request re-optimization.
                        std::cout << "hadd Sources and Target have different "
                                     "compression levels"
                                  << std::endl;
                        std::cout << "hadd merging will be slower" << std::endl;
                    }
                }
                merger.SetNotrees(noTrees);
                merger.SetMergeOptions(cacheSize);
                merger.SetIOFeatures(features);
                Bool_t status;
                if (append)
                    status = merger.PartialMerge(TFileMerger::kIncremental | TFileMerger::kAll);
                else
                    status = merger.Merge();
                return status;
            };

            auto sequentialMerge = [&](TFileMerger &merger, size_t start, size_t nFiles) {
                std::cout << "Starting the sequential merge" << std::endl;
                for (auto i = start; i < (start + nFiles) && i < outFiles.size(); ++i)
                {
                    std::ifstream indirect_file(outFiles.at(i));
                    if (!indirect_file.is_open())
                    {
                        std::cerr << "hadd could not open file " << std::endl;
                        throw std::runtime_error("hadd was unable to merge the file\n");
                    }
                    if (!merger.AddFile(outFiles.at(i).c_str()))
                    {
                        if (skip_errors)
                        {
                            std::cerr << "hadd skipping file with error: " << outFiles.at(i)
                                      << std::endl;
                        }
                        else
                        {
                            std::cerr << "hadd exiting due to error in " << outFiles.at(i)
                                      << std::endl;
                            throw std::runtime_error("hadd was unable to merge the file\n");
                        }
                    }
                    else
                        std::cout << "Added file: " << outFiles.at(i) << std::endl;
                }
                std::cout << "Finished the sequential merge" << std::endl;
                return mergeFiles(merger);
            };

            auto parallelMerge = [&](int start) {
                std::cout << "Starting the parallel merge" << std::endl;
                TFileMerger mergerP(kFALSE, kFALSE);
                mergerP.SetMsgPrefix("hadd");
                mergerP.SetPrintLevel(verbosity - 1);
                if (maxopenedfiles > 0)
                {
                    mergerP.SetMaxOpenedFiles(static_cast<int>(maxopenedfiles / nProcesses));
                }
                if (!mergerP.OutputFile(partialFiles[start / step].c_str(), newcomp))
                {
                    std::cerr << "hadd error opening target partial file" << std::endl;
                    throw std::runtime_error("hadd was unable to merge the file\n");
                }
                std::cout << "Finished the parallel merge" << std::endl;
                return sequentialMerge(mergerP, start, step);
            };

            auto reductionFunc = [&]() {
                for (const auto &pf : partialFiles)
                {
                    fileMerger.AddFile(pf.c_str());
                }
                return mergeFiles(fileMerger);
            };

            if (multiproc)
            {
                // ROOT::TProcessExecutor p(nProcesses);
                // auto                   res = p.Map(parallelMerge,
                //                                    ROOT::TSeqI(0, outFiles.size(), step));
                // auto status = std::accumulate(res.begin(), res.end(), 0U)
                //            == partialFiles.size();
                std::vector<std::thread> threads;
                for (unsigned int i{0}; i < nProcesses; ++i)
                    threads.emplace_back(parallelMerge, i * step);
                for (auto &it : threads)
                    it.join();

                auto status = reductionFunc();
                if (!status)
                    throw std::runtime_error("Wrong exit status in reductionFunc\n");
                // if (!debug) {
                //    for (const auto &pf : partialFiles) {
                //       gSystem->Unlink(pf.c_str());
                //    }
                // }
            }
            else
            {
                std::cout << "Calling sequential merge" << std::endl;
                auto status = sequentialMerge(fileMerger, 0, filesToProcess);
                if (!status)
                    throw std::runtime_error("Wrong exit status in reductionFunc\n");
            }

            if (verbosity > 1)
            {
                std::cout << "hadd merged " << fileMerger.GetMergeList()->GetEntries()
                          << " input files in " << outFileName << "\n";
            }
            {
                std::string rmCmd{"rm -f"};
                for (const auto &it : partialFiles)
                {
                    rmCmd += " " + it;
                }
                // std::cout << "CMD: " << rmCmd << std::endl;
                if (system(rmCmd.c_str()) != 0)
                    printf("The %s (Remove partial files) returned bad value\n", rmCmd.c_str());
            }
        }
        if (settings.remove_partial)
        {
            std::string rmCmd{"rm -f"};
            for (const auto &it : outFiles)
            {
                rmCmd += " " + it;
            }
            // std::cout << "CMD: " << rmCmd << std::endl;
            if (system(rmCmd.c_str()) != 0)
                printf("The %s (Remove partial files) returned bad value\n", rmCmd.c_str());
        }
        finalizeSumJob(id, validJob, Jobs::Status::FINISHED);
        timer.Stop();
        timer.Print();
    }
}

void Worker::updatePrisma(const std::string &replayFilePath,
                          const std::string &replayFileName,
                          const std::string &prismaFilePath)
{
    if (settings.verbosity_level > 0)
    {
        std::cout << "\nUpdating Prisma analyzed branches\n";
        std::cout << "Replay path: " << replayFilePath << "\n";
        std::cout << "Replay file: " << replayFileName << "\n";
        std::cout << "Prisma path: " << prismaFilePath << std::endl << std::endl;
        std::cout << "Prisma file pattern: " << conf.prisma.prisma_file_pattern << std::endl
                  << std::endl;
    }

    // prisma chain, check if open
    TChain priTree("PrismaTree");

    DIR                     *dir = nullptr;
    struct dirent           *ent;
    std::vector<std::string> fileList;
    if ((dir = opendir(prismaFilePath.c_str())) != nullptr)
    {
        // Get all files in the requested directory
        while ((ent = readdir(dir)) != nullptr)
        {
            fileList.emplace_back(ent->d_name);
            // printf ("%s\n", ent->d_name);
        }
        std::sort(fileList.begin(), fileList.end());
        for (size_t fileNr{0}; fileNr < fileList.size(); ++fileNr)
        {
            // Check if the file pattern matches the required one
            if (fileList[fileNr].compare(0, conf.prisma.prisma_file_pattern.size(),
                                         conf.prisma.prisma_file_pattern) == 0 &&
                fileList[fileNr].substr(fileList[fileNr].find_last_of('.') + 1) == "root")
            {
                if (settings.verbosity_level > 1)
                    std::cout << "Found Prisma File " << fileList[fileNr] << std::endl;
                priTree.Add((prismaFilePath + fileList[fileNr]).c_str());
            }
        }
    }
    else
    {
        throw std::runtime_error("Could not open directory :" + prismaFilePath);
    }

    if (priTree.FindBranch("Analyzed") == nullptr)
        throw std::runtime_error("PrismaTree does not contain Analyzed branch");

    struct stat p_statbuf{};
    //   if (lstat((replayFilePath + "old_" + replayFileName).c_str(),
    //   &p_statbuf) > 0) {
    lstat((replayFilePath + "old_" + replayFileName).c_str(), &p_statbuf);
    // std::cout << "old_" << replayFileName << (S_ISLNK(p_statbuf.st_mode)==1
    // ? " is SYMLINK" : " is FILE")  << std::endl;
    if (S_ISLNK(p_statbuf.st_mode) == 1)
    {
        if (system(("unlink " + replayFilePath + "old_" + replayFileName).c_str()) != 0)
        {
            throw std::runtime_error("System call failed: " + std::string("unlink ") +
                                     replayFilePath + std::string("old_") + replayFileName);
        }
    }
    //   }
    struct stat p2_statbuf{};
    //    if (lstat((replayFilePath + replayFileName).c_str(), &p2_statbuf) >
    //    0) {
    lstat((replayFilePath + replayFileName).c_str(), &p2_statbuf);
    if (S_ISLNK(p2_statbuf.st_mode) == 1)
    {
        if (system(("unlink " + replayFilePath + replayFileName).c_str()) != 0)
        {
            throw std::runtime_error("System call failed: " + std::string("unlink ") +
                                     replayFilePath + replayFileName);
        }
        std::ifstream f((replayFilePath + "old_" + replayFileName).c_str());
        std::ifstream f2((replayFilePath + "new_" + replayFileName).c_str());
        if (!f.good() && f2.good())
        {
            // std::cout<< replayFilePath << "old_" << replayFileName << " -> "
            // << (f.good() ? "OK" : "NOT OK") << std::endl; std::cout<<
            // replayFilePath << "new_" << replayFileName << " -> " <<
            // (f2.good() ? "OK" : "NOT OK") << std::endl;
            if (system(("mv " + replayFilePath + "new_" + replayFileName + " " + replayFilePath +
                        "old_" + replayFileName)
                           .c_str()) != 0)
            {
                throw std::runtime_error("System call failed: " + std::string("mv ") +
                                         replayFilePath + std::string("new_ ") + replayFileName +
                                         std::string(" ") + replayFilePath + std::string("old_") +
                                         replayFileName);
            };
            //                system(("ln -s " + replayFilePath +"old_" +
            //                replayFileName + " " + replayFilePath +
            //                replayFileName).c_str());
        }
        f.close();
        f2.close();
    }
    //    }
    // if update was not done already, change names of the files
    std::ifstream fin((replayFilePath + "old_" + replayFileName).c_str());
    if (!fin.good())
    {
        if (settings.verbosity_level > 0)
        {
            std::cout << "File " << (replayFilePath + "old_" + replayFileName) << " not found,\n"
                      << "moving " << (replayFilePath + replayFileName) << " into "
                      << (replayFilePath + "old_" + replayFileName) << std::endl;
        }
        std::string cmd1{"mv "};
        cmd1 += replayFilePath;
        cmd1 += replayFileName;
        cmd1 += " ";
        cmd1 += replayFilePath;
        cmd1 += "old_";
        cmd1 += replayFileName;
        if (system(cmd1.c_str()))
            throw std::runtime_error("Unable to move file in --update_prisma\n");
        std::string cmd2{"cd "};
        cmd2 += replayFilePath;
        cmd2 += "&& ln -sf ";
        cmd2 += "old_";
        cmd2 += replayFileName;
        cmd2 += " ";
        cmd2 += replayFileName;
        cmd2 += "&& cd -";
        if (system(cmd2.c_str()))
            throw std::runtime_error("Unable to link old file in --update_prisma\n");
    }

    // replay file, check if open
    TFile repFile((replayFilePath + "old_" + replayFileName).c_str(), "read");

    if (!repFile.IsOpen())
        throw std::runtime_error("Could not find selected TFile\n");
    if (repFile.IsZombie())
        throw std::runtime_error("TFile not closed and cannot be recovered\n");
    // if(repFile.TestBit(TFile::kRecovered)) {}; // It's ok!

    // replay tree, check if open
    auto *repTree = dynamic_cast<TTree *>(repFile.Get(conf.replay.tree_name.c_str()));
    if (repTree == nullptr)
        throw std::runtime_error("Could not find TTree in TFile\n");

    // check if structure is correct
    bool priAnaBranchPresent = true;
    if (repTree->FindBranch("MONITOR_1") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("MONITOR_2") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("MCP_X") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("MCP_Y") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("MCP_Q") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("MCP_Theta") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("MCP_Phi") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("X_FP") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("Y_FP") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("TOF") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("IC_E") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("IC_DE_A") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("IC_DE_AB") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("IC_RANGE") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("IC_Drift_B") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("IC_Pads") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("Theta") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("Phi") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("Beta") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("Length") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("Radius") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("RBeta") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("A_over_q") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("Mass") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("Qvalue") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("Theta_BP") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("Phi_BP") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("Beta_BP") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("TAC_LT_LTS") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("IC_a_numpads") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("IC_b_numpads") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("IC_c_numpads") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("IC_d_numpads") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("IC_col_a") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("IC_col_b") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("IC_col_c") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("IC_col_d") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("Z_Nr") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("Q_Nr") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("A_Nr") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("mcp_ok") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("tof_ok") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("traj_ok") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("side_ok") == nullptr)
        priAnaBranchPresent = false;
    if (repTree->FindBranch("ic_ok") == nullptr)
        priAnaBranchPresent = false;
    if (!priAnaBranchPresent)
        throw std::runtime_error("TreeMaster structure for Prisma analyzed branches");

    // output file, check if open
    TFile newFile((replayFilePath + "new_" + replayFileName).c_str(), "recreate");
    if (!newFile.IsOpen())
        throw std::runtime_error("Could not create new TFile\n");
    if (newFile.IsZombie())
        throw std::runtime_error("new TFile IsZombie\n");

    // output tree, check if open
    newFile.cd();
    TTree *newTree = repTree->CloneTree(0);

    if (settings.verbosity_level > 1)
    {
        std::cout << "Opening input and output files - successfull" << std::endl;
    }

    ULong64_t TSPrismaPri, TSPrismaRep;

    repTree->SetBranchAddress("TSPrisma", &TSPrismaRep);
    priTree.SetBranchAddress("TSPrisma", &TSPrismaPri);

    // Binary search
    // Objective: skip time corresponding to other femul trees

    if (settings.verbosity_level > 0)
    {
        std::cout << "Performing binary search algorithm" << std::endl;
    }

    Long64_t  idxR          = 0;
    Long64_t  idxP          = 0;
    ULong64_t TotalMatching = 0;

    repTree->SetBranchStatus("*", false);
    repTree->SetBranchStatus("TSPrisma", true);

    priTree.SetBranchStatus("*", false);
    priTree.SetBranchStatus("TSPrisma", true);

    // skip zeroes in replay tree
    repTree->GetEntry(idxR);
    while (TSPrismaRep == 0)
    {
        idxR++;
        repTree->GetEntry(idxR);
    }

    Long64_t lowP  = 0;
    Long64_t midP  = 0;
    auto     highP = priTree.GetEntries();

    if (settings.verbosity_level > 1)
    {
        std::cout << "Initial conditions:\tIdxR " << idxR << "\tidxP " << idxP << "\tlowP " << lowP
                  << "\thighP " << highP << "\tpriTS " << TSPrismaPri << "\trepTS " << TSPrismaRep
                  << std::endl;
    }

    while (true)
    {
        // get mid point and skip zeroes
        midP = (lowP + highP) / 2;
        priTree.GetEntry(midP);

        if (settings.verbosity_level > 1)
        {
            std::cout << "Status:\tlowP " << lowP << "\tmidP " << midP << "\thighP " << highP
                      << "\tpriTS " << TSPrismaPri << "\trepTS " << TSPrismaRep << std::endl;
        }

        if (lowP == midP || midP == highP)
            break; // avoid infinite loop

        // binary search logic
        if (TSPrismaPri - conf.prisma.ts_correct > TSPrismaRep)
        {
            highP = midP;
        }
        if (TSPrismaPri - conf.prisma.ts_correct < TSPrismaRep)
        {
            lowP = midP;
        }
        if (TSPrismaPri - conf.prisma.ts_correct == TSPrismaRep)
        {
            lowP = midP;
            break;
        }
    }
    idxP = lowP;

    repTree->SetBranchStatus("*", true);
    priTree.SetBranchStatus("*", true);

    if (settings.verbosity_level > 1)
    {
        std::cout << "Status after Binary Search:\tIdxR " << idxR << "\tidxP " << idxP << "\tlowP "
                  << lowP << "\thighP " << highP << "\tpriTS " << TSPrismaPri << "\trepTS "
                  << TSPrismaRep << std::endl;
    }

    auto repTreeEntries = repTree->GetEntries();

    if (settings.verbosity_level > 0)
    {
        std::cout << "Matching events with same timestamp" << std::endl;
    }

    // WARNING:
    // the prismaAnalyzed variable is shared by all three trees (priTree,
    // repTree, newTree) i.e. the function GetEntry overwrites the variable, so
    // the order of the calls of GetEntry is very important

    // restart replay index from 0
    idxR = 0;

    struct PrismaAnalyzed
    {
        Float_t MONITOR_1;
        Float_t MONITOR_2;
        Float_t MCP_X;
        Float_t MCP_Y;
        Float_t MCP_Q;
        Float_t MCP_Theta;
        Float_t MCP_Phi;
        Float_t X_FP;
        Float_t Y_FP;
        Float_t TOF;
        Float_t TOF_EXT;
        Float_t IC_Pads[40];
        Float_t IC_E;
        Float_t IC_DE_A;
        Float_t IC_DE_AB;
        Float_t IC_DE_AB_COR;
        Float_t IC_RANGE;
        Float_t IC_Drift_A;
        Float_t IC_Drift_B;
        Float_t IC_Drift_C;
        Float_t Theta;
        Float_t Phi;
        Float_t Beta;
        Float_t Length;
        Float_t Radius;
        Float_t RBeta;
        Float_t A_over_q_uncal;
        Float_t A_over_q;
        Float_t Mass;
        Float_t Qvalue;
        Float_t Theta_BP;
        Float_t Phi_BP;
        Float_t Beta_BP;
        Float_t TAC_LT_LTS;
        UChar_t IC_COL_A;
        UChar_t IC_COL_B;
        UChar_t IC_COL_C;
        UChar_t IC_COL_D;
        UChar_t IC_a_numpads;
        UChar_t IC_b_numpads;
        UChar_t IC_c_numpads;
        UChar_t IC_d_numpads;
        UChar_t Z_Nr;
        UChar_t Q_Nr;
        UChar_t A_Nr;
        UChar_t mcp_ok;
        UChar_t tof_ok;
        UChar_t traj_ok;
        UChar_t side_ok;
        UChar_t ic_ok;
        UChar_t z_ok;
        UChar_t q_ok;
        UChar_t a_ok;
    } prismaAnalyzed{};

    // read prisma analyzed branch from priTree
    priTree.SetBranchAddress("Analyzed", &prismaAnalyzed);

    // read prisma analyzed branches from repTree
    repTree->SetBranchAddress("MONITOR_1", &prismaAnalyzed.MONITOR_1);
    repTree->SetBranchAddress("MONITOR_2", &prismaAnalyzed.MONITOR_2);
    repTree->SetBranchAddress("MCP_X", &prismaAnalyzed.MCP_X);
    repTree->SetBranchAddress("MCP_Y", &prismaAnalyzed.MCP_Y);
    repTree->SetBranchAddress("MCP_Q", &prismaAnalyzed.MCP_Q);
    repTree->SetBranchAddress("MCP_Theta", &prismaAnalyzed.MCP_Theta);
    repTree->SetBranchAddress("MCP_Phi", &prismaAnalyzed.MCP_Phi);
    repTree->SetBranchAddress("X_FP", &prismaAnalyzed.X_FP);
    repTree->SetBranchAddress("Y_FP", &prismaAnalyzed.Y_FP);
    repTree->SetBranchAddress("TOF", &prismaAnalyzed.TOF);
    repTree->SetBranchAddress("TOF_ext", &prismaAnalyzed.TOF_EXT);
    repTree->SetBranchAddress("IC_Pads", prismaAnalyzed.IC_Pads);
    repTree->SetBranchAddress("IC_E", &prismaAnalyzed.IC_E);
    repTree->SetBranchAddress("IC_DE_A", &prismaAnalyzed.IC_DE_A);
    repTree->SetBranchAddress("IC_DE_AB", &prismaAnalyzed.IC_DE_AB);
    repTree->SetBranchAddress("IC_RANGE", &prismaAnalyzed.IC_RANGE);
    repTree->SetBranchAddress("IC_Drift_A", &prismaAnalyzed.IC_Drift_A);
    repTree->SetBranchAddress("IC_Drift_B", &prismaAnalyzed.IC_Drift_B);
    repTree->SetBranchAddress("IC_Drift_C", &prismaAnalyzed.IC_Drift_C);
    repTree->SetBranchAddress("IC_col_a", &prismaAnalyzed.IC_COL_A);
    repTree->SetBranchAddress("IC_col_b", &prismaAnalyzed.IC_COL_B);
    repTree->SetBranchAddress("IC_col_c", &prismaAnalyzed.IC_COL_C);
    repTree->SetBranchAddress("IC_col_d", &prismaAnalyzed.IC_COL_D);
    repTree->SetBranchAddress("IC_a_numpads", &prismaAnalyzed.IC_a_numpads);
    repTree->SetBranchAddress("IC_b_numpads", &prismaAnalyzed.IC_b_numpads);
    repTree->SetBranchAddress("IC_c_numpads", &prismaAnalyzed.IC_c_numpads);
    repTree->SetBranchAddress("IC_d_numpads", &prismaAnalyzed.IC_d_numpads);
    repTree->SetBranchAddress("Theta", &prismaAnalyzed.Theta);
    repTree->SetBranchAddress("Phi", &prismaAnalyzed.Phi);
    repTree->SetBranchAddress("Beta", &prismaAnalyzed.Beta);
    repTree->SetBranchAddress("Length", &prismaAnalyzed.Length);
    repTree->SetBranchAddress("Radius", &prismaAnalyzed.Radius);
    repTree->SetBranchAddress("RBeta", &prismaAnalyzed.RBeta);
    repTree->SetBranchAddress("A_over_q", &prismaAnalyzed.A_over_q);
    repTree->SetBranchAddress("Mass", &prismaAnalyzed.Mass);
    repTree->SetBranchAddress("Qvalue", &prismaAnalyzed.Qvalue);
    repTree->SetBranchAddress("Theta_BP", &prismaAnalyzed.Theta_BP);
    repTree->SetBranchAddress("Phi_BP", &prismaAnalyzed.Phi_BP);
    repTree->SetBranchAddress("Beta_BP", &prismaAnalyzed.Beta_BP);
    repTree->SetBranchAddress("TAC_LT_LTS", &prismaAnalyzed.TAC_LT_LTS);
    repTree->SetBranchAddress("Z_Nr", &prismaAnalyzed.Z_Nr);
    repTree->SetBranchAddress("Q_Nr", &prismaAnalyzed.Q_Nr);
    repTree->SetBranchAddress("A_Nr", &prismaAnalyzed.A_Nr);
    repTree->SetBranchAddress("mcp_ok", &prismaAnalyzed.mcp_ok);
    repTree->SetBranchAddress("tof_ok", &prismaAnalyzed.tof_ok);
    repTree->SetBranchAddress("traj_ok", &prismaAnalyzed.traj_ok);
    repTree->SetBranchAddress("side_ok", &prismaAnalyzed.side_ok);
    repTree->SetBranchAddress("ic_ok", &prismaAnalyzed.ic_ok);
    repTree->SetBranchAddress("z_ok", &prismaAnalyzed.z_ok);
    repTree->SetBranchAddress("q_ok", &prismaAnalyzed.q_ok);
    repTree->SetBranchAddress("a_ok", &prismaAnalyzed.a_ok);

    // write prisma analyzed variables on newTree
    newTree->SetBranchAddress("MONITOR_1", &prismaAnalyzed.MONITOR_1);
    newTree->SetBranchAddress("MONITOR_2", &prismaAnalyzed.MONITOR_2);
    newTree->SetBranchAddress("MCP_X", &prismaAnalyzed.MCP_X);
    newTree->SetBranchAddress("MCP_Y", &prismaAnalyzed.MCP_Y);
    newTree->SetBranchAddress("MCP_Q", &prismaAnalyzed.MCP_Q);
    newTree->SetBranchAddress("MCP_Theta", &prismaAnalyzed.MCP_Theta);
    newTree->SetBranchAddress("MCP_Phi", &prismaAnalyzed.MCP_Phi);
    newTree->SetBranchAddress("X_FP", &prismaAnalyzed.X_FP);
    newTree->SetBranchAddress("Y_FP", &prismaAnalyzed.Y_FP);
    newTree->SetBranchAddress("TOF", &prismaAnalyzed.TOF);
    newTree->SetBranchAddress("TOF_ext", &prismaAnalyzed.TOF_EXT);
    newTree->SetBranchAddress("IC_Pads", prismaAnalyzed.IC_Pads);
    newTree->SetBranchAddress("IC_E", &prismaAnalyzed.IC_E);
    newTree->SetBranchAddress("IC_DE_A", &prismaAnalyzed.IC_DE_A);
    newTree->SetBranchAddress("IC_DE_AB", &prismaAnalyzed.IC_DE_AB);
    newTree->SetBranchAddress("IC_RANGE", &prismaAnalyzed.IC_RANGE);
    newTree->SetBranchAddress("IC_Drift_A", &prismaAnalyzed.IC_Drift_A);
    newTree->SetBranchAddress("IC_Drift_B", &prismaAnalyzed.IC_Drift_B);
    newTree->SetBranchAddress("IC_Drift_C", &prismaAnalyzed.IC_Drift_C);
    newTree->SetBranchAddress("IC_col_a", &prismaAnalyzed.IC_COL_A);
    newTree->SetBranchAddress("IC_col_b", &prismaAnalyzed.IC_COL_B);
    newTree->SetBranchAddress("IC_col_c", &prismaAnalyzed.IC_COL_C);
    newTree->SetBranchAddress("IC_col_d", &prismaAnalyzed.IC_COL_D);
    newTree->SetBranchAddress("IC_a_numpads", &prismaAnalyzed.IC_a_numpads);
    newTree->SetBranchAddress("IC_b_numpads", &prismaAnalyzed.IC_b_numpads);
    newTree->SetBranchAddress("IC_c_numpads", &prismaAnalyzed.IC_c_numpads);
    newTree->SetBranchAddress("IC_d_numpads", &prismaAnalyzed.IC_d_numpads);
    newTree->SetBranchAddress("Theta", &prismaAnalyzed.Theta);
    newTree->SetBranchAddress("Phi", &prismaAnalyzed.Phi);
    newTree->SetBranchAddress("Beta", &prismaAnalyzed.Beta);
    newTree->SetBranchAddress("Length", &prismaAnalyzed.Length);
    newTree->SetBranchAddress("Radius", &prismaAnalyzed.Radius);
    newTree->SetBranchAddress("RBeta", &prismaAnalyzed.RBeta);
    newTree->SetBranchAddress("A_over_q", &prismaAnalyzed.A_over_q);
    newTree->SetBranchAddress("Mass", &prismaAnalyzed.Mass);
    newTree->SetBranchAddress("Qvalue", &prismaAnalyzed.Qvalue);
    newTree->SetBranchAddress("Theta_BP", &prismaAnalyzed.Theta_BP);
    newTree->SetBranchAddress("Phi_BP", &prismaAnalyzed.Phi_BP);
    newTree->SetBranchAddress("Beta_BP", &prismaAnalyzed.Beta_BP);
    newTree->SetBranchAddress("TAC_LT_LTS", &prismaAnalyzed.TAC_LT_LTS);
    newTree->SetBranchAddress("Z_Nr", &prismaAnalyzed.Z_Nr);
    newTree->SetBranchAddress("Q_Nr", &prismaAnalyzed.Q_Nr);
    newTree->SetBranchAddress("A_Nr", &prismaAnalyzed.A_Nr);
    newTree->SetBranchAddress("mcp_ok", &prismaAnalyzed.mcp_ok);
    newTree->SetBranchAddress("tof_ok", &prismaAnalyzed.tof_ok);
    newTree->SetBranchAddress("traj_ok", &prismaAnalyzed.traj_ok);
    newTree->SetBranchAddress("side_ok", &prismaAnalyzed.side_ok);
    newTree->SetBranchAddress("ic_ok", &prismaAnalyzed.ic_ok);
    newTree->SetBranchAddress("z_ok", &prismaAnalyzed.z_ok);
    newTree->SetBranchAddress("q_ok", &prismaAnalyzed.q_ok);
    newTree->SetBranchAddress("a_ok", &prismaAnalyzed.a_ok);

    // Compare and fill algorithm
    while (true)
    {
        // calculate progress
        if (idxR % (repTreeEntries / 1000) == 0)
            progress.store((100. * static_cast<double>(idxR)) /
                           static_cast<double>(repTreeEntries));

        if (settings.verbosity_level > 1 && idxR % 10000 == 0)
            std::cout << "Processed " << idxR << " events out of " << repTreeEntries << "\t("
                      << TotalMatching << " matching events)\tprogress " << std::fixed
                      << std::setprecision(1) << progress << " %\n";

        if (!repTree->GetEntry(idxR))
            break; // first read replay

        // save replay events without prisma
        while (TSPrismaRep == 0)
        {
            newTree->Fill();
            idxR++;
            if (!repTree->GetEntry(idxR))
                break;
        }

        if (!priTree.GetEntry(idxP))
            break; // second read prisma

        // find matching timestamps logic
        if (TSPrismaPri - conf.prisma.ts_correct < TSPrismaRep)
        {
            idxP++;
            continue;
        }
        if (TSPrismaPri - conf.prisma.ts_correct > TSPrismaRep)
        {
            idxR++;
            continue;
        }
        if (TSPrismaPri - conf.prisma.ts_correct == TSPrismaRep)
        { // matching timestamps
            newTree->Fill();
            idxP++;
            idxR++;
            TotalMatching++;
        }
    }

    if (settings.verbosity_level > 0)
    {
        std::cout << "Total Matching Events: " << TotalMatching << std::endl;
    }

    newTree->Write();
    newFile.Write();
    newFile.Close();

    {
        std::string cmd{"cd "};
        cmd += replayFilePath;
        cmd += "&& ln -sf ";
        cmd += "new_";
        cmd += replayFileName;
        cmd += " ";
        cmd += replayFileName;
        cmd += "&& cd -";
        if (system(cmd.c_str()))
            throw std::runtime_error("Unable to link new file in --update_prisma\n");
    }

    if (settings.verbosity_level > 0)
    {
        std::cout << "Output file " << replayFilePath + "new_" + replayFileName
                  << " written and closed" << std::endl;
    }

    if (settings.verbosity_level > 1)
    {
        std::cout << "Deleting replay tree" << std::endl;
    }
    delete repTree;
}

WorkerSpy::WorkerSpy(const unsigned int &id) : WorkerBase(id), outFile("tmp.root", "recreate")
{
    if (settings.verbosity_level > 0)
    {
        std::cout << "WorkerSpy created with id: " << id << std::endl;
    }
    // Create a tmp_enabled_histos file
    auto &modsettings = Settings::getInstance();
    auto &modconf     = ConfManager::getInstance();

    if (!settings.spy.empty())
    {
        modsettings.only_enabled_histos        = true;
        modsettings.nrThr                      = 1;
        modconf.replay.enabled_histo_file_name = "tmp_enabled_histos.txt";
        std::ofstream tmpFile(conf.replay.conf_path + "/" + conf.replay.enabled_histo_file_name);
        // Strip the required histo from the last back slash
        auto lastSlash = modsettings.spy.find_last_of("/");
        if (lastSlash == std::string::npos)
        {
            std::cerr << "Could not find last slash in: " << settings.spy << std::endl;
            return;
        }
        tmpFile << settings.spy.substr(lastSlash + 1) << std::endl;
        tmpFile.close();
    }

    outFile.cd();
    if (settings.userSelector)
    {
        selector = new UserSelector(outFile);
    }
    else
    {
        selector = new AgataSelector(outFile);
    }
    selector->ConfProgress(&progress);
    selector->SetOption("");
}

void WorkerSpy::Sel()
{
    while (true)
    {
        auto [validJob, j] = getJob(id);
        // auto getJobReturn = getJob(id);
        // auto validJob{getJobReturn.first};
        // auto j{getJobReturn.second};

        // Check if StopFile is present
        {
            std::ifstream f("StopFile");
            if (f.good())
                return;
        }

        if (validJob < 0)
            break;
        // std::cout << "[SLAVE] Received valid job: " << j.outFileName <<
        // std::endl;

        std::string const inFileNameTotal =
            (j.inPath.empty() || j.inPath[j.inPath.length() - 1] == '/')
                ? j.inPath + j.inFileName
                : j.inPath + "/" + j.inFileName;
        std::string const outFileNameTotal =
            (j.outPath.empty() || j.outPath[j.outPath.length() - 1] == '/')
                ? j.outPath + j.outFileName
                : j.outPath + "/" + j.outFileName;

        TFile *file = nullptr;
        file        = new TFile(inFileNameTotal.c_str(), "read");

        if (!file->IsOpen())
            throw std::runtime_error("Could not find selected TFile\n");
        if (file->IsZombie())
            throw std::runtime_error("TFile not closed and cannot be recovered\n");
        // if(file->TestBit(TFile::kRecovered)) {}; // It's ok!

        auto *tree = dynamic_cast<TTree *>(file->Get(conf.replay.tree_name.c_str()));
        if (tree == nullptr)
            throw std::runtime_error("Could not find TTree in TFile\n");

        selector->SetFirstTS(j.initialTimestamp);
        selector->SetJobType(j.type);

        tree->SetNotify(selector);

        if (!initialized)
        {
            try
            {
                Initialize(tree);
            }
            catch (const MemCheck::OutOfMemoryException &e)
            {
                std::cerr << "Using too many threads given the available memory. Trying to manage "
                             "the threads"
                          << std::endl;
                std::cerr << e.what() << std::endl;
                file->Close();
                delete file;
                finalizeJob(id, validJob, Jobs::Status::MEMORYERROR); // Unable to finish job
                sleep(2);
                // continue;
                return;
            }
        }

        LoadTree(tree);

        int       batchSize = 1000;
        long long entries   = tree->GetEntries();
        long long i         = 0;
        do
        {
            long long firstentry = i;
            long long lastentry  = i + batchSize;
            if (settings.verbosity_level > 1)
            {
                std::cout << "Processing entries " << i << " to " << i + batchSize << std::endl;
            }
            if (lastentry > entries)
                lastentry = entries;
            ProcessEvents(firstentry, lastentry, tree);
            i += batchSize;
        } while (i < entries);
        file->Close();
        if (settings.verbosity_level > 1)
        {
            std::cout << "File closed, deleting" << std::endl;
        }
        delete file;

        finalizeJob(id, validJob, Jobs::Status::FINISHED); // Finished job correctly
    }
    terminate.store(true);
}

void WorkerSpy::Initialize(TTree *tree)
{
    outFile.cd();
    selector->Begin(tree);      //<===call user initialization function
    selector->SlaveBegin(tree); //<===call user initialization function
    initialized = true;
}

void WorkerSpy::LoadTree(TTree *tree)
{
    if (selector->Version() >= 2)
        selector->Init(tree);
    selector->Notify();
}

void WorkerSpy::ProcessEvents(const long long &firstentry, const long long &lastentry, TTree *tree)
{
    // TThread::Lock();
    for (long long entry = firstentry; entry < lastentry; entry++)
    {
        Long64_t treeEntry = tree->GetEntryNumber(entry);
        if (treeEntry < 0)
            break;
        tree->LoadTree(treeEntry);
        selector->Process(treeEntry); // Process each event and update histograms
        if (selector->GetAbort() == TSelector::kAbortProcess)
            break;
    }
    // TThread::UnLock();
    // std::this_thread::sleep_for(std::chrono::milliseconds(1));
}
