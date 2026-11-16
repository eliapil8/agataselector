#include "LauncherDistributed.h"

#include <algorithm>

#include <boost/mpi.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>

LauncherDistributed::LauncherDistributed()
    : world(MpiManager::getInstance().world) {}

void LauncherDistributed::Manager() {
    if(MpiManager::getInstance().world.rank() == 0)
        Distribute<MpiManager::tags::JOB1, Worker::Jobs>(jobs);
    else
        CommWithMaster();
}

void LauncherDistributed::SumManager() {
    if(MpiManager::getInstance().world.rank() == 0)
        Distribute<MpiManager::tags::JOB2, std::vector<Worker::Jobs>>(jobsToAdd);
    else
        return;
    // CommWithMaster();
}

std::pair<int, Worker::Jobs>
LauncherDistributed::getJob(const unsigned int& id) {
    return Retrieve<MpiManager::tags::JOB1, Worker::Jobs>(id);
}

std::pair<int, std::vector<Worker::Jobs>>
LauncherDistributed::getSumJobs(const unsigned int& id) {
    return Retrieve<MpiManager::tags::JOB2, std::vector<Worker::Jobs>>(id);
}

void LauncherDistributed::finalizeSumJobs(const unsigned int& id, const int& uniqueJobId, const Worker::Jobs::Status& status) {
    //Launcher::finalizeSumJobs(id, uniqueJobId, status);
    finalize(id, uniqueJobId, status);
}

void LauncherDistributed::finalizeJob(const unsigned int& id, const int& uniqueJobId, const Worker::Jobs::Status& status) {
    //Launcher::finalizeJob(id, uniqueJobId, status);
    finalize(id, uniqueJobId, status);
}

void LauncherDistributed::finalize(const unsigned int& id, const int& uniqueJobId, const Worker::Jobs::Status& status) {
    auto& c = MpiManager::getInstance().GetComm(id);
    switch(status){
    case Worker::Jobs::Status::FINISHED:
        c.send(0, MpiManager::tags::STATUS_UPDATE, MpiManager::status::FINISHED);
        break;
    case Worker::Jobs::Status::MEMORYERROR:
        c.send(0, MpiManager::tags::STATUS_UPDATE, MpiManager::status::ERROR);
        break;
    case Worker::Jobs::ERROR:
        throw std::runtime_error("Unhandled error received by mpi");
        break;
    }
    c.send(0, MpiManager::tags::JOB_ID, uniqueJobId);
    world.send(0, MpiManager::tags::ACKNOWLEDGE, id);
}

std::map<int, std::map<int, double>> LauncherDistributed::getProgress() {
    assert(world.rank() == 0);
    for(int i{1}; i < world.size(); ++i) {
        world.send(i, MpiManager::tags::PROGRESS);
    }
    std::map<int, std::map<int, double>> returnVal;
    for(int i{1}; i < world.size(); ++i) {
        world.recv(i, MpiManager::tags::PROGRESS, returnVal[i]);
    }
    return returnVal;
}

void LauncherDistributed::CommWithMaster() {
    if(settings.verbosity_level != 0) return;
    assert(world.rank() != 0);
    boost::mpi::request killReq = world.irecv(0, MpiManager::tags::KILL);
    while(!killReq.test()) {
        world.recv(0, MpiManager::tags::PROGRESS);
        auto res = Launcher::getProgress();
        world.send(0, MpiManager::tags::PROGRESS, res.at(0));
    }
}

template <MpiManager::tags tag, class J>
std::pair<int, J> LauncherDistributed::Retrieve(const unsigned int& id) {
    boost::mpi::communicator& c = MpiManager::getInstance().GetComm(id);
    // std::cout << "[SLAVE: " << world.rank()
    //           << "] worker "<<id<<" Sending ready signal to MASTER. comm"
    //           "rank is "<< c.rank() <<"/"<<c.size() << std::endl;

    // std::stringstream log;
    c.send(0, MpiManager::tags::STATUS_UPDATE, MpiManager::status::READY);
    world.send(0, MpiManager::tags::ACKNOWLEDGE, id);

    // Check if a new job is coming
    bool stop;
    auto recvTest = c.recv(0, MpiManager::tags::KILL, stop);
    if(stop) {
        // std::cout << "[SLAVE: " << world.rank()
        //           << "] worker " <<id <<" Received a kill signal from MASTER"
        //           "that has rank " << recvTest.source() << std::endl;
        c.send(0, MpiManager::tags::STATUS_UPDATE, MpiManager::status::KILLED);
        world.send(0, MpiManager::tags::ACKNOWLEDGE, id);
        return {-1, {}};
    }

    std::pair<int, J> job;
    c.recv(0, tag, job.second);
    c.recv(0, MpiManager::JOB_ID, job.first);
    //std::cout << "[SLAVE: " << world.rank()
    //          << "] Sending job to worker " << id <<" with tag "<< tag 
    //          << " job id: " << job.first
    //          << std::endl;

    c.send(0, MpiManager::tags::STATUS_UPDATE, MpiManager::status::WORKING);
    world.send(0, MpiManager::tags::ACKNOWLEDGE, id);

    return job;
}

template <MpiManager::tags tag, class J>
void LauncherDistributed::Distribute(std::deque<Assignments<J>>& jobs) {
    std::map<std::pair<int, int>, bool> totalWorkers;

    do {
        int                worker{-1};
        // Blocking communication to save cpu time and wait from a signal from
        // a worker
        boost::mpi::status msg = world.recv(
            boost::mpi::any_source, MpiManager::tags::ACKNOWLEDGE, worker);
        int   node{msg.source()};
        // std::cout << "Message from node " << node << ", tag " << msg.tag()
        // << " related to worker " << worker << std::endl;

        auto& c = MpiManager::getInstance().GetComm(
            worker); // Gets the right communicator for a worker
        boost::optional<boost::mpi::status> test = c.probe();
        MpiManager::tags tagTmp{static_cast<MpiManager::tags>(test->tag())};
        // std::cout << "Reading node " << node << " and worker " << worker <<
        // " with tag " << tagTmp <<std::endl;
        switch(tagTmp) {
        case MpiManager::tags::STATUS_UPDATE: {
            MpiManager::status status{MpiManager::status::UNKNOWN};
            c.recv(node, MpiManager::tags::STATUS_UPDATE, status);
            totalWorkers[std::pair<int, int>(node, worker)] = true;
            switch(status) {
            case MpiManager::status::READY:{
                totalWorkers[std::pair<int, int>(node, worker)] = true;
                auto job = Launcher::getGeneralJob<J>(node, jobs, jobMtx);
                if(job.first > 0) {
                    c.isend(node, MpiManager::tags::KILL, false);
                    c.isend(node, tag, job.second);
                    c.isend(node, MpiManager::tags::JOB_ID, job.first);
                } else {
                    c.isend(node, MpiManager::tags::KILL, true);
                    //c.isend(node, MpiManager::tags::KILL, true);
                }
                break;
            }
            case MpiManager::status::WORKING: break;
            case MpiManager::status::FINISHED: {
                int uniqueJobId{-1};
                c.recv(node, MpiManager::tags::JOB_ID, uniqueJobId);
                finalizeGeneralJob<J>(node, uniqueJobId, Worker::Jobs::Status::FINISHED, jobs, jobMtx);
                break;
            }
            case MpiManager::status::ERROR: {
                int uniqueJobId{-1};
                c.recv(node, MpiManager::tags::JOB_ID, uniqueJobId);
                finalizeGeneralJob<J>(node, uniqueJobId, Worker::Jobs::Status::MEMORYERROR, jobs, jobMtx);
                break;
            }
            case MpiManager::status::KILLED:
                totalWorkers.erase(std::pair<int, int>(node, worker));
                break;
            default:
                throw std::runtime_error(
                    "Node has returned a nonsense status\n");
            }
            break;
        }
        case MpiManager::tags::PROGRESS:
            std::cerr
                << "Should not be here, was an acknowledge wrongly called? "
                << std::endl;
            assert(tagTmp == MpiManager::tags::PROGRESS);
            break;

        case MpiManager::tags::LOG:
            std::cerr << "Not implemented " << std::endl;
            assert(tagTmp == MpiManager::tags::LOG);
            break;

        default:
            throw std::runtime_error(
                "Worker threads should not send other tags, received "
                + std::to_string(tagTmp) + "\n");
        }
    } while(!totalWorkers.empty());

    // Send kill to world
    for(int i{1}; i < world.size(); ++i) {
        //std::cout << "Sending kill signal to world rank " << i <<std::endl; 
        world.send(i, MpiManager::tags::KILL);
    }
    
}

void LauncherDistributed::SumAll() const {
    if(MpiManager::getInstance().world.rank() == 0) Launcher::SumAll();
}
