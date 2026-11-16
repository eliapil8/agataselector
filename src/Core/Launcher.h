#pragma once

#include <TApplication.h>
#include <TFile.h>
#include <TTree.h>

#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <dirent.h>

#include "AgataSelector.h"
#include "ConfManager.h"
#include "Settings.h"
#include "Worker.h"

class Launcher {
  public:
    Launcher();
    virtual ~Launcher() {
        //for(auto& it1: progress)
        //    for(auto& it2: it1.second)
        //        delete it2.second;
    };

    virtual void                          Setup();
    virtual void                          Manager();
    virtual void                          SumManager();
    virtual void                          SumAll() const;
    /** @return Full path of the file produced by SumAll(), or empty if unavailable. */
    std::string                           GetSumAllFileName() const;

    virtual std::pair<int, Worker::Jobs> getJob(const unsigned int&);
    virtual std::pair<int, std::vector<Worker::Jobs>>
                             getSumJobs(const unsigned int&);
    virtual void             finalizeJob(const unsigned int&, const int&, const Worker::Jobs::Status&);
    virtual void             finalizeSumJobs(const unsigned int&, const int&, const Worker::Jobs::Status&);
    std::vector<std::string> GetListOfFinalFiles() const;

    virtual size_t           JobsInStack();
    virtual std::map<int, std::map<int, double>>       getProgress();

    std::map<int, std::map<int, std::atomic<double>*>> progress;

  public:
    const Settings&                       settings;
    const ConfManager&                    conf;

    template<class J>
    struct Assignments{
        enum Status{
            ASSIGNED,
            UNASSIGNED,
            COMPLETED,
            TOREPEAT,
            ERROR,
        };
        int uniqueId;
        static int nextJobId;
        Status status;
        J job;
        explicit Assignments(const J& j): uniqueId(nextJobId++), status(Status::UNASSIGNED), job(j){};
    };
    std::deque<Assignments<Worker::Jobs>>              jobs;
    std::deque<Assignments<std::vector<Worker::Jobs>>> jobsToAdd;
    std::vector<Worker::Jobs>             sumAll;
    std::vector<std::string>              listOfFinalFiles;

    std::mutex                            jobMtx;
    static bool FileExists(const std::string& fileName) {
        std::ifstream f(fileName.c_str());
        if(!f.good()) return false;

        TFile testFile(fileName.c_str(), "read");

        // If the file is ok and finished, add it to jobs to be performed
        if(testFile.IsOpen()
           && !testFile.IsZombie() /*!testFile.TestBit(TFile::kRecovered)*/)
            return true;
        else
            return false;
    }
    template <class J>
    static std::pair<int, J> getGeneralJob(const unsigned int& id, std::deque<Assignments<J>>& jobs, std::mutex& mtx){
        (void)id;
        std::lock_guard<std::mutex> const guard(mtx);

        //Remove finished jobs
        jobs.erase(
            std::remove_if(jobs.begin(), jobs.end(),
                           [](const auto& val) {
                             return val.status == Assignments<J>::Status::COMPLETED;
                           }),
            jobs.end()
        );

        //If all jobs are finished send communication of their status
        if(jobs.empty()) return {-1, {}};

        //Find unassigned job
        auto job = std::find_if(jobs.begin(), jobs.end(),
                                [](const auto& val) {
                                  return val.status == Assignments<J>::Status::UNASSIGNED;
                                });
        if(job != jobs.end()){
            job->status = Assignments<J>::Status::ASSIGNED;
            return{job->uniqueId, job->job};
        };

        //Find unassigned job
        job = std::find_if(jobs.begin(), jobs.end(),
                                [](const auto& val) {
                                    return val.status == Assignments<J>::Status::TOREPEAT;
                                });
        if(job != jobs.end()){
            job->status = Assignments<J>::Status::ASSIGNED;
            sleep(rand() % 20 + 1); //Sleep for a random amount up to 20 s
            return{job->uniqueId, job->job};
        };

        //Otherwise return dummy job
        return {-1, {}};
    }
    template <class J>
    static void finalizeGeneralJob(const unsigned int& id, const int& jobUniqueId, const Worker::Jobs::Status& status, std::deque<Assignments<J>>& jobs, std::mutex& mtx){
        (void)id;
        std::lock_guard<std::mutex> const guard(mtx);

        switch(status) {
        case Worker::Jobs::Status::FINISHED:
            //Mark as finished
            for(auto &it: jobs){
                if(it.uniqueId == jobUniqueId){
                    it.status = Assignments<J>::Status::COMPLETED;
                    return;
                }
            }
            break;
        case Worker::Jobs::Status::MEMORYERROR:
            //Back in the queue
            for(auto &it: jobs){
                if(it.uniqueId == jobUniqueId){
                    it.status = Assignments<J>::Status::TOREPEAT;
                    return;
                }
            }
            break;
        case Worker::Jobs::Status::ERROR:
            throw std::runtime_error("Error status in job");
        }
        throw std::runtime_error("Job not found in stack, something is wrong");
    }

  private:
    static unsigned long long FindFirstTS(TTree*) ;
  protected:
};
