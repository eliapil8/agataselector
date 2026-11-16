#include "App.h"

#include <csignal>
#include <iostream>
#include <list>
#include <memory>
#include <thread>
#include <vector>

#ifndef MAC_OSX
#include "sys/sysinfo.h"
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "ConfManager.h"
#include "EdECalculator/EdECalculatorOscar.h"
#include "Launcher.h"
#include "Print.h"
#include "Settings.h"
#include "Worker.h"

#include <TSystem.h>
#include "TApplication.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TThread.h"
#include "TTree.h"
#include "TRint.h"

#ifdef USE_MPI
#include "LauncherDistributed.h"
#include "MpiManager.h"

#include <boost/mpi/communicator.hpp>
#endif

void App::sigintFunction(int) {
    std::cout << "\tCreating stop file to close nicely the root files"
              << std::endl;
    std::ofstream output("StopFile");
}

App::App() {
    // Tell root to protect for multithreading
    ROOT::EnableThreadSafety();

    Settings& settings = Settings::getInstance();

#ifndef GRID_OPTIMIZER
    ConfManager& conf = ConfManager::getInstance();
    conf.Initialize();
#endif

#ifdef USE_MPI
    boost::mpi::communicator& world = MpiManager::getInstance().world;
    world.barrier();
#endif

    // Handle ctrl+c
    struct sigaction sigIntHandler {};
    sigIntHandler.sa_handler = sigintFunction;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, nullptr);

    Launcher* launcher{nullptr};
    if(!settings.distribute) {
        if(settings.verbosity_level > 0)
            std::cout << "Instantiating Launcher" << std::endl;
        launcher = new Launcher();
        launcher->Setup();
    } else {
#ifdef USE_MPI
        if(settings.verbosity_level > 0)
            std::cout << "Instantiating distributed Launcher" << std::endl;
        launcher = new LauncherDistributed();
        if(world.rank() == 0) { // Is master machine
            launcher->Setup();
        } else { /*Is worker machine*/
        }
#else
        std::cerr << "Compile with the USE_MPI option to access this feature"
                  << std::endl;
        assert(settings.distribute == false);
#endif
    }

#ifdef USE_MPI
    if(settings.distribute) {
        // Generate different communicators for different threads
        world.barrier();
        for(unsigned int i{0}; i < settings.nrThr; ++i) {
            if(settings.distribute) { MpiManager::getInstance().Split(i); }
        }
    }
#endif

    if(settings.verbosity_level > 0)
        std::cout << "Starting threads" << std::endl;
    std::vector<std::thread> threads;
    std::list<WorkerBase*>   workers;
    std::thread              manager(&Launcher::Manager, launcher);

    if(settings.verbosity_level > 0) {
        MemCheck::printStats();
    }

    for(unsigned int i{0}; i < settings.nrThr; ++i) {
        int cpu{0};
#ifdef USE_MPI
        if(settings.distribute && world.rank() == 0)
            continue; // No workers for master node
#endif
        //Create the correct type of worker
        if(settings.spy.empty()) {
            workers.emplace_back(new Worker(i));
        } else {
            workers.emplace_back(new WorkerSpy(i));
        }

        //Initialize the worker
        launcher->progress[cpu].emplace(i, &workers.back()->progress);
        workers.back()->getJob
            = std::bind(&Launcher::getJob, launcher, std::placeholders::_1);
        workers.back()->finalizeJob = std::bind(
            &Launcher::finalizeJob, launcher, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

        //Start the job here, everything should be initialized
        if(settings.spy.empty()) {
            threads.emplace_back(&Worker::Sel, static_cast<Worker*>(workers.back()));
        } else {
            threads.emplace_back(&WorkerSpy::Sel, static_cast<WorkerSpy*>(workers.back()));
        }

        //Print the memory usage
        if(settings.verbosity_level > 0) {
            MemCheck::printStats();
        }
    }

    std::function<std::map<int, std::map<int, double>>()> askProgress
        = std::bind(&Launcher::getProgress, launcher);

    bool spawnPrint{settings.verbosity_level == 0};
#ifdef USE_MPI
    if(settings.distribute && world.rank() != 0) spawnPrint = false;
#endif

    Print*            print{nullptr};
    std::thread*      printThread{nullptr};
    std::atomic<bool> terminate{false};
    if(spawnPrint) {
        print
            = new Print(terminate, std::bind(&Launcher::JobsInStack, launcher),
                        askProgress);
        printThread = new std::thread(&Print::printProgress, print);
    }

    if(!settings.spy.empty()) {
        if(workers.size() != 1) throw std::runtime_error("Number of workers for spy does not make sense");

        auto worker = dynamic_cast<WorkerSpy*>(workers.front());
        if(worker == nullptr) throw std::runtime_error("Could not cast worker to WorkerSpy");
        Spy(worker->outFile, worker->terminate);
    }
    for(auto& it : threads) it.join();
    manager.join();
    terminate.store(true);
    if(printThread != nullptr) printThread->join();
    delete print;
    delete printThread;

    threads.clear();
    if(settings.verbosity_level > 0) {
        std::cout << "Selector threads have joined " << std::endl;
    }
#ifdef USE_MPI
    if(settings.distribute) world.barrier();
#endif

    unsigned int nrWorkersSum{0};
    std::thread  sumManager(&Launcher::SumManager, launcher);
    auto         workersHadd{settings.nrThr / 4};
    if(workersHadd == 0) workersHadd = 1;
    if(settings.verbosity_level > 0) {
        std::cout << "Starting to spawn workers for hadd" << std::endl;
    }
    for(auto& worker : workers) {
        if(nrWorkersSum++ == workersHadd) break;
        worker->getSumJob      = std::bind(&Launcher::getSumJobs, launcher,
                                          std::placeholders::_1);
        worker->finalizeSumJob = std::bind(&Launcher::finalizeSumJobs, launcher,
                                          std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        if(settings.spy.empty())
            threads.emplace_back(&Worker::Sum, static_cast<Worker*>(worker));
        else
            threads.emplace_back(&WorkerSpy::Sum, static_cast<WorkerSpy*>(worker));
    }

    for(auto& it : threads) it.join();
    sumManager.join();
    finalFiles = launcher->GetListOfFinalFiles();

    std::vector<std::string> canvasFiles = finalFiles;
    if(settings.sum_all) {
        launcher->SumAll();
        canvasFiles = {launcher->GetSumAllFileName()};
    }

    if(settings.spy.empty() && !settings.update_prisma
       && settings.reduction_condition.empty()) {
        WriteOscarEdECanvases(canvasFiles);
    }

    for(auto worker : workers) delete worker;
    delete launcher;
}

void App::WriteOscarEdECanvases(const std::vector<std::string>& files) const {
    const auto& conf     = ConfManager::getInstance();
    const auto& settings = Settings::getInstance();

    struct OscarCanvasJob {
        std::string                          directory;
        std::unique_ptr<EdECalculatorOscar> calculator;
    };
    std::vector<OscarCanvasJob> jobs;

    auto addJob = [&](const std::string& directory, const OscarConf& oscarConf, bool present) {
        if(!present || !oscarConf.edeKinematicLines) return;
        if(!oscarConf.dE_E_Histos)
            throw EdECalculator::EdEConfigurationError(
                "EDE_KINEMATIC_LINES requires DE_E_HISTOS YES for " + directory);
        if(oscarConf.edeReactionFile.empty())
            throw EdECalculator::EdEConfigurationError("EDE_REACTION_FILE is empty for "
                                                        + directory);

        auto calculator = std::make_unique<EdECalculatorOscar>(
            conf.reaction, conf.target, conf.replay, oscarConf);
        std::string reactionFile = oscarConf.edeReactionFile;
        if(reactionFile.front() != '/')
            reactionFile = conf.replay.conf_path + "/" + reactionFile;
        calculator->LoadReactionFile(reactionFile);
        calculator->BuildGraphs();
        jobs.push_back({directory, std::move(calculator)});
    };

    addJob("OscarUp", conf.oscarUp, conf.detectorsPresent.OscarUp);
    addJob("OscarDown", conf.oscarDown, conf.detectorsPresent.OscarDown);
    if(jobs.empty()) return;

    for(const auto& fileName : files) {
        if(fileName.empty()) continue;
        if(settings.verbosity_level > 0)
            std::cout << "Writing OSCAR E-dE canvases to merged file " << fileName << std::endl;

        TFile file(fileName.c_str(), "UPDATE");
        if(!file.IsOpen() || file.IsZombie() || !file.IsWritable())
            throw std::runtime_error("Unable to open merged ROOT file for OSCAR E-dE canvases: "
                                     + fileName);
        for(const auto& job : jobs)
            job.calculator->WriteCanvases(file, job.directory);
        file.Flush();
        file.Close();
    }
}

void App::Spy(TFile& outFile, std::atomic<bool>& terminate) {
    auto interface = new TRint("App", nullptr, nullptr);
    interface->SetPrompt("\u001b[1;32mcmd [%d]\u001b[1;0m ");

    // make sure that the Gpad and GUI libs are loaded
    TRint::NeedGraphicsLibs();
    TApplication::NeedGraphicsLibs();
    gApplication->InitializeGraphics();
    TCanvas cv;

    //Loop until the worker thread is done
    const Settings& settings = Settings::getInstance();

    TObject* histo{nullptr};
    while(!terminate.load()) {
        // Lock, update the canvas, and unlock
        //TThread::Lock();
        if(histo == nullptr){
            std::cout << "Getting histo: " << settings.spy << std::endl;
            histo = outFile.Get(settings.spy.c_str());
            if(histo != nullptr) histo->Draw();
        }else{
            std::cout << "Redrawing histo: " << settings.spy << std::endl;
            cv.Modified();
            cv.Update();
        }
        //TThread::UnLock();

        // Process GUI events to keep the canvas responsive
        gSystem->ProcessEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    //histo->Clone()->Draw();
    cv.Modified();
    cv.Update();
    gSystem->ProcessEvents();

    interface->Run();

    return;

}


//void App::StartSpy() {
//    Settings& settings = Settings::getInstance();
//    ConfManager& conf = ConfManager::getInstance();
//
//    //Grab the ttree
//    TFile* file = TFile::Open(settings.spyFiles.first.c_str(), "READ");
//    if(file == nullptr) {
//        std::cerr << "Could not open file: " << settings.spyFiles.first
//                  << std::endl;
//        return;
//    }
//    auto* tree = dynamic_cast<TTree*>(file->Get(conf.replay.tree_name.c_str()));
//    if(tree == nullptr) {
//        std::cerr << "Could not find tree: " << conf.replay.tree_name
//                  << " in file: " << settings.spyFiles.first << std::endl;
//        return;
//    }
//
//    //Create a tmp_enabled_histos file
//    settings.only_enabled_histos = true;
//    conf.replay.enabled_histo_file_name = "tmp_enabled_histos.txt";
//    std::ofstream tmpFile(conf.replay.conf_path +"/"+conf.replay.enabled_histo_file_name);
//    //Strip the required histo from the last back slash
//    auto lastSlash = settings.spyFiles.second.find_last_of("/");
//    if(lastSlash == std::string::npos) {
//        std::cerr << "Could not find last slash in: " << settings.spyFiles.second
//                  << std::endl;
//        return;
//    }
//    tmpFile << settings.spyFiles.second.substr(lastSlash + 1) << std::endl;
//    tmpFile.close();
//
//    //Set up the launcher
//    LauncherSpy launcher;
//    launcher.Setup();
//    launcher.SetupTree(tree, 0);
//    auto& outFile = launcher.GetOutFile();
//
//    std::atomic<bool> terminate{false};
//    auto fillFunction = [&](){
//      int nperbatch{10000};
//      for(long long i{0}; i < tree->GetEntries(); i += nperbatch) {
//          // Acquire ROOT’s global lock before touching ROOT objects
//          TThread::Lock();
//          auto last = i + nperbatch;
//          if(last > tree->GetEntries()) last = tree->GetEntries();
//          launcher.ProcessEvents(i, last);
//
//          // Release the lock
//          TThread::UnLock();
//      }
//      terminate.store(true);
//      std::cout << "Done filling" << std::endl;
//    };
//    std::thread worker(fillFunction);
//
//    auto interface = new TRint("App", nullptr, nullptr);
//    interface->SetPrompt("\u001b[1;32mcmd [%d]\u001b[1;0m ");
//
//    // make sure that the Gpad and GUI libs are loaded
//    TRint::NeedGraphicsLibs();
//    TApplication::NeedGraphicsLibs();
//    gApplication->InitializeGraphics();
//    TCanvas cv;
//
//    //Loop until the worker thread is done
//    auto* histo = (TH1*)outFile.Get(settings.spyFiles.second.c_str());
//    if(histo == nullptr)
//        throw std::runtime_error("Could not find histogram: " + settings.spyFiles.second);
//
//    histo->Draw();
//    while(!terminate.load()) {
//        // Lock, update the canvas, and unlock
//        TThread::Lock();
//        cv.Modified();
//        cv.Update();
//        TThread::UnLock();
//
//        // Process GUI events to keep the canvas responsive
//        std::this_thread::sleep_for(std::chrono::milliseconds(300));
//        gSystem->ProcessEvents();
//    }
//    //histo->Clone()->Draw();
//    cv.Modified();
//    cv.Update();
//    gSystem->ProcessEvents();
//
//    interface->Run();
//
//    launcher.Finalize();
//    return;
//
//}
