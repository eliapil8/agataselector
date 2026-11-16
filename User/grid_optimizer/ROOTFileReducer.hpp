#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

#include <csignal>
#include <sys/wait.h>
#include <unistd.h>

#include "TFile.h"
#include "TTree.h"

namespace reducer
{

class ROOTFileReducer;

struct alignas(64) ReductionJob // add necessary padding to prevent cache misses
{
    std::string              in_file_name;
    std::string              out_file_name;
    std::string              reduction_condition;
    std::string              tree_name;
    std::vector<std::string> disable_branches;

    ReductionJob(const std::string &inFileName, const std::string &outFileName, const std::string &reductionCondition,
                 const std::string &treeName, const std::vector<std::string> &disabledBranches)
        : in_file_name(inFileName), out_file_name(outFileName), reduction_condition(reductionCondition), tree_name(treeName),
          disable_branches(disabledBranches)
    {
        assert(!in_file_name.empty() && "Input file name is empty");
        assert(!out_file_name.empty() && "Output file name is empty");
        assert(!reduction_condition.empty() && "Reduction condition is empty");
        assert(!tree_name.empty() && "Tree name is empty");
    };

    ReductionJob()                     = default;
    ReductionJob(const ReductionJob &) = default;
    ReductionJob(ReductionJob &&)      = default;
    ReductionJob &operator=(const ReductionJob &) = default;
};

class ROOTFileReducer
{

  private:
    std::mutex                fManagerMtx;
    std::vector<ReductionJob> fReductionJobs;

  private:
    /// @brief Check if all jobs are unique to avoid reading or writing same files.
    /// @return true if unique, false otherwise
    bool AreJobsUnique();
    /// @brief actual worker that does the reductiom, it is called by Reduce() in a forked process
    /// @param job_start iterator to mark the start of the jobs to be reduced (included)
    /// @param job_end iterator to mark the end of the jobs to be reduced (excluded)
    void SubReduce(std::vector<reducer::ReductionJob>::iterator job_start, std::vector<reducer::ReductionJob>::iterator job_end);

  public:
    void AddReductionJob(ReductionJob &&job) { fReductionJobs.emplace_back(job); };
    bool GetJob(ReductionJob &job);
    void Reduce();
};


bool ROOTFileReducer::AreJobsUnique()
{
    for (unsigned int i = 0; i < fReductionJobs.size(); i++)
    {
        for (unsigned int j = i + 1; j < fReductionJobs.size(); j++)
        {
            if (fReductionJobs[i].in_file_name == fReductionJobs[j].in_file_name)
            {
                std::cout << "File " << fReductionJobs[i].in_file_name << " is present more than once in the list of files to be reduced"
                          << std::endl;
                return false;
            }
            if (fReductionJobs[i].out_file_name == fReductionJobs[j].out_file_name)
            {
                std::cout << "File " << fReductionJobs[i].out_file_name << " is present more than once in the list of files to be reduced"
                          << std::endl;
                return false;
            }
        }
    }
    return true;
}


void ROOTFileReducer::SubReduce(std::vector<reducer::ReductionJob>::iterator job_start, std::vector<reducer::ReductionJob>::iterator job_end)
{
    for(auto job_it = job_start; job_it != job_end; ++job_it)
    {
        std::cout << "Reducing file " << job_it->in_file_name << " to " << job_it->out_file_name << std::endl;
        TFile full_file(job_it->in_file_name.c_str(), "READ");

        auto *tree = dynamic_cast<TTree *>(full_file.Get(job_it->tree_name.c_str()));
        assert(tree != nullptr && "Could not find TTree in TFile\n");

        for (const auto &disable_branch : job_it->disable_branches) { tree->SetBranchStatus(disable_branch.c_str(), 0); }

        TFile reducedFile(job_it->out_file_name.c_str(), "RECREATE");
        assert(!reducedFile.IsZombie() && "Could not create new TFile\n");
        assert(reducedFile.IsOpen() && "new TFile IsZombie\n");

        TTree *reducedTree = tree->CopyTree(job_it->reduction_condition.c_str());

        reducedTree->Write();
        reducedFile.Write();
        reducedFile.Close();
    }

}


void ROOTFileReducer::Reduce()
{
    assert(AreJobsUnique() && "Jobs are not unique");
    static const unsigned int nthreads = std::thread::hardware_concurrency();
    if(fReductionJobs.size()>0)
    {
        std::cout << "Starting reduction of " << fReductionJobs.size() << " files" << std::endl;
    }
    else
    {
        std::cout << "No files to reduce!" << std::endl;
        return;
    }
    
    unsigned int total_forks    = fReductionJobs.size() > nthreads ? nthreads : fReductionJobs.size();
    int          activeChildren = static_cast<int>(total_forks);

    for (unsigned int nfork = 0; nfork < total_forks; ++nfork)
    {
        unsigned int start = (nfork * fReductionJobs.size() / total_forks);
        unsigned int stop  = (nfork + 1) * fReductionJobs.size() / total_forks > fReductionJobs.size()
                                ? fReductionJobs.size()
                                : (nfork + 1) * fReductionJobs.size() / total_forks;
        std::vector<ReductionJob>::iterator jobs_start = fReductionJobs.begin() + start;
        std::vector<ReductionJob>::iterator jobs_end   = fReductionJobs.begin() + stop;

        pid_t childPid = fork();

        if (childPid == -1) { std::runtime_error("Fork failed"); }
        if (childPid == 0)
        {
            SubReduce(jobs_start, jobs_end);
            exit(0);
        }
    }

    // Wait for all child processes to terminate
    int   status;
    pid_t terminatedChild;
    while (activeChildren > 0)
    {
        terminatedChild = waitpid(-1, &status, 0);
        if (terminatedChild > 0)
        {
            std::cout << "Parent process: Child " << terminatedChild << " terminated.\n";
            activeChildren--;
        }
    }
}

bool ROOTFileReducer::GetJob(ReductionJob &job)
{
    std::lock_guard<std::mutex> lock(fManagerMtx);
    if (fReductionJobs.empty()) return false;
    job = fReductionJobs.back();
    fReductionJobs.pop_back();
    return true;
}

} // namespace reducer