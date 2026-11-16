#pragma once

#include <mutex>
#include <string>
#include <utility>
#include <atomic>

#include <TMemFile.h>

#include "ConfManager.h"
#include "DetectorAnalysis/DetectorAnalysis.h"
#include "Settings.h"

#ifdef USE_MPI
#include <boost/serialization/string.hpp>
#endif

class AgataSelector;

//Implement a virtual base class for the worker
class WorkerBase {
  protected:
    explicit WorkerBase(const unsigned int& id) : id(id), settings(Settings::getInstance()), conf(ConfManager::getInstance()) {};
    const unsigned int id;
    const Settings&    settings;
    const ConfManager& conf;

  public:
    virtual ~WorkerBase() = default;
    WorkerBase() = delete;
    WorkerBase(const WorkerBase&) = delete;
    WorkerBase(WorkerBase&&) = delete;
    WorkerBase& operator=(const WorkerBase&) = delete;
    WorkerBase& operator=(WorkerBase&&) = delete;

  public:
    virtual void Sel() = 0;
    virtual void Sum() = 0;
    
  public:
    struct Jobs {
        int                            runID{};
        std::string                    inFileName;
        std::string                    inPath;
        std::string                    outFileName;
        std::string                    outPath;
        unsigned long long             initialTimestamp{};
        enum Status{
            FINISHED,
            MEMORYERROR,
            ERROR,
        };
        DetectorAnalysis::Inputs::Type type;
        Jobs(const int& ID, std::string inFileName, std::string inPath,
             std::string outFileName, std::string outPath,
             const unsigned long long&             initTS,
             const DetectorAnalysis::Inputs::Type& t)
            : runID(ID), inFileName(std::move(inFileName)),
              inPath(std::move(inPath)), outFileName(std::move(outFileName)),
              outPath(std::move(outPath)), initialTimestamp(initTS), type(t) {};
        Jobs()=default;

#ifdef USE_MPI
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            (void)version;
            ar & runID;
            ar & inFileName;
            ar & inPath;
            ar & outFileName;
            ar & outPath;
            ar & initialTimestamp;
            ar & type;
        }
#endif
    };

    std::atomic<double>                      progress{0};
    std::function<std::pair<int, Jobs>(const unsigned int&)> getJob;
    std::function<std::pair<int, std::vector<Jobs>>(const unsigned int&)>
                                             getSumJob;
    std::function<void(const unsigned int&, const int&, const WorkerBase::Jobs::Status&)> finalizeJob;
    std::function<void(const unsigned int&, const int&, const WorkerBase::Jobs::Status&)> finalizeSumJob;
};

class Worker : public WorkerBase {
  public:
    explicit Worker(const unsigned int& id) : WorkerBase(id) {};
    // No copy of move (because atomic variables are a mess)
    Worker(const Worker&)            = delete;
    Worker(Worker&&)                 = delete;
    Worker& operator=(const Worker&) = delete;
    Worker& operator=(Worker&&)      = delete;
    ~Worker() override = default;

    void Sel() override;
    void Sum() override;



  private:
    void updatePrisma(const std::string& replayFilePath,
                      const std::string& replayFileName,
                      const std::string& prismaFilePath);
};


class WorkerSpy : public WorkerBase{
  public:
    explicit WorkerSpy(const unsigned int& id);
    // No copy of move (because atomic variables are a mess)
    WorkerSpy(const WorkerSpy&)            = delete;
    WorkerSpy(WorkerSpy&&)                 = delete;
    WorkerSpy& operator=(const WorkerSpy&) = delete;
    WorkerSpy& operator=(WorkerSpy&&)      = delete;
    ~WorkerSpy() noexcept override = default;

    void Sel() override;
    void Sum() override {};

    TMemFile outFile;
    AgataSelector* selector;
    std::atomic<bool> terminate{false};
    bool initialized{false};

  public:
    void Initialize(TTree* tree);
    void LoadTree(TTree* tree);
    void ProcessEvents(const long long& start, const long long& end, TTree* tree);
};