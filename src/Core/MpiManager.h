#pragma once

#include <map>
#include <mutex>

#include <boost/mpi/communicator.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/status.hpp>

class MpiManager {
  public:
    static MpiManager& getInstance() {
        static MpiManager instance;
        return instance;
    }

    inline boost::mpi::communicator& GetComm(const unsigned int& id) {
        auto c = comm.find(id);
        if(c != comm.end())
            return c->second;
        else {
            throw std::runtime_error("Communicator " + std::to_string(id)
                                     + " not found\n");
        }
    }

    inline void Split(const int& id) {
        std::lock_guard<std::mutex> lock(splitMtx);
        comm.emplace(id, world.split(id, world.rank()));
    }

    boost::mpi::environment  env;
    boost::mpi::communicator world;

    MpiManager(const MpiManager&)            = delete;
    MpiManager& operator=(const MpiManager&) = delete;

    enum tags { STATUS_UPDATE, PROGRESS, JOB1, JOB2, LOG, ACKNOWLEDGE, KILL, JOB_ID };

    enum status { READY, WORKING, FINISHED, KILLED, UNKNOWN, ERROR };

  private:
    MpiManager() : env(boost::mpi::threading::multiple) {
        if(boost::mpi::environment::thread_level()
           < boost::mpi::threading::multiple)
            throw std::runtime_error(
                "Multithreading in boost mpi not supported but needed\n");
    };
    ~MpiManager() = default;
    std::mutex                              splitMtx;
    std::map<int, boost::mpi::communicator> comm;
};