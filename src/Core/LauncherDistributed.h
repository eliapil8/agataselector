#pragma once

#include <mutex>

#include "Launcher.h"
#include "MpiManager.h"

#include <boost/mpi/communicator.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/status.hpp>

class LauncherDistributed : public Launcher {
  public:
    LauncherDistributed();
    ~LauncherDistributed() override = default;

    void                          Manager() override;
    void                          SumManager() override;
    void                          SumAll() const override;
    std::pair<int, Worker::Jobs> getJob(const unsigned int&) override;
    std::pair<int, std::vector<Worker::Jobs>>
         getSumJobs(const unsigned int&) override;
    void finalizeJob(const unsigned int&, const int&, const Worker::Jobs::Status&) override;
    void finalizeSumJobs(const unsigned int&, const int&, const Worker::Jobs::Status&) override;

    // int JobsInStack() override;
    std::map<int, std::map<int, double>> getProgress() override;

  private:
    void finalize(const unsigned int&, const int& uniqueJobId, const Worker::Jobs::Status& status);
    template <MpiManager::tags tag, class J> void Distribute(std::deque<Assignments<J>>&);

    void                                              CommWithMaster();

    template <MpiManager::tags tag, class J>
    std::pair<int, J>        Retrieve(const unsigned int& id);

    boost::mpi::communicator& world;
    // std::map<int, boost::mpi::communicator> comm;
    // std::map<int, int> masterRank;
    // std::map<int, int> commMaster;
    // std::mutex commIns;
    // std::mutex rankIns;
};
