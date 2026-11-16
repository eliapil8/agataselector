#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

#include <csignal>
#include <filesystem>
#include <sys/wait.h>
#include <type_traits>
#include <unistd.h>

#include "TFile.h"
#include "TTree.h"

namespace reducer
{

template <typename T>
// cpp20 version:
//
// requires requires(T a, T b) { { a == b } -> std::same_as<bool>; }
// bool HaveDuplicates(std::vector<T> v1, std::vector<T> v2)
typename std::enable_if<std::is_convertible<decltype(std::declval<T>() == std::declval<T>()), bool>::value, bool>::type HaveDuplicates(
    std::vector<T> v1, std::vector<T> v2)
{
    for (const auto &v1_elem : v1)
    {
        for (const auto &v2_elem : v2)
        {
            if (v1_elem == v2_elem) return true;
        }
    }
    return false;
}

struct alignas(64) MergeJob // add necessary padding to prevent cache misses
{
    std::vector<std::string> in_file_names;
    std::string              out_file_name;

    MergeJob(const std::vector<std::string> &inFileName, const std::string &outFileName)
        : in_file_names(inFileName), out_file_name(outFileName)
    {
        std::cout << "Creating merge job with " << in_file_names.size() << " files" << std::endl;
        assert(!in_file_names.empty() && "Input file list is empty");
        assert(!out_file_name.empty() && "Output file name is empty");
    };

    MergeJob()                 = default;
    MergeJob(const MergeJob &) = default;
    MergeJob(MergeJob &&)      = default;
    MergeJob &operator=(const MergeJob &) = default;

    float GetJobSize_MB() const
    {
        float size_MB = 0;
        for (const auto &file_name : in_file_names) { size_MB += std::filesystem::file_size(file_name) / 1024. / 1024.; }
        return size_MB;
    }
    unsigned int GetNFiles() const { return in_file_names.size(); }
};

class ROOTFileMerger
{

  private:
    std::mutex            fManagerMtx;
    std::vector<MergeJob> fMergeJobs;
    float                 fMaxFileSizeMB = 1000;
    unsigned int          fThreads;

  private:
    /// @brief Check if all jobs are unique to avoid reading or writing same files.
    /// @return true if unique, false otherwise
    bool AreJobsUnique();
    /// @brief actual worker that does the reductiom, it is called by Reduce() in a forked process
    /// @param job_start iterator to mark the start of the jobs to be reduced (included)
    /// @param job_end iterator to mark the end of the jobs to be reduced (excluded)
    void SubMerge(std::vector<reducer::MergeJob>::iterator job_start, std::vector<reducer::MergeJob>::iterator job_end);
    void OptimizeMergeJobs();

  public:
    ROOTFileMerger(unsigned int nthreads) noexcept : fThreads(nthreads){};
    void AddMergeJob(MergeJob &&job) noexcept { fMergeJobs.emplace_back(job); };
    void Merge();
    void SetMaxFileSize_MB(float max_file_size) { fMaxFileSizeMB = max_file_size; };
    void AddAllInDir(const std::string &directory_path, const std::string &out_file_name);
};

void ROOTFileMerger::OptimizeMergeJobs()
{
    // std::vector<MergeJob> optimized_jobs;

    // unsigned int njobs = fMergeJobs.size();
    // /// how many set of jobs we will create, should be divisible by number of threads available
    // unsigned int nsets     = njobs / fThreads + (njobs % fThreads == 0 ? 0 : 1);
    // unsigned int free_jobs = njobs % fThreads;

    // unsigned int fixed_jobs = 0;

    // // arbitrary
    // float minimal_size_MB = 100;
    // float optimal_size_MB = 200;

    // std::vector<float> job_sizes_MB;
    // std::for_each(fMergeJobs.begin(), fMergeJobs.end(), [&job_sizes_MB](const auto &job) { job_sizes_MB.push_back(job.GetJobSize_MB()); });
    // std::sort(fMergeJobs.begin(), fMergeJobs.end(),
    //           [](const auto &job1, const auto &job2) { return job1.GetJobSize_MB() > job2.GetJobSize_MB(); });

    // // let's move all jobs that are smaller then optmial
    // for (unsigned int i = fMergeJobs.size() - 1; i >= 0; i--)
    // {
    //     if (fMergeJobs[i].GetJobSize_MB() < minimal_size_MB)
    //     {
    //         optimized_jobs.push_back(fMergeJobs[i]);
    //         fMergeJobs.pop_back();
    //         fixed_jobs++;
    //     }
    //     else
    //         break;
    // }

    // int njobs_needed = 0;

    // std::vector<float> distance_from_optimal;
    // for (auto &job : fMergeJobs)
    // {
    //     int _njobs = job.GetJobSize_MB() / optimal_size_MB;
    //     njobs_needed += njobs;
    //     distance_from_optimal.push_back(job.GetJobSize_MB() - njobs_needed * optimal_size_MB);
    // }

    // lets see how much jobs we need to have everyone below
}

void ROOTFileMerger::AddAllInDir(const std::string &directory_path, const std::string &out_file_name)
{
    std::vector<std::string> rootFiles;
    for (const auto &entry : std::filesystem::directory_iterator(directory_path))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".root")
        {
            // auto size_in_MB = std::filesystem::file_size(entry.path()) / 1024. / 1024.;
            rootFiles.push_back(entry.path().string());
        }
    }
    if(rootFiles.empty())
    {
        std::cout << "Directory " << directory_path << " does not contain any files, skipping output file " << out_file_name << std::endl;
        return;
    }
    fMergeJobs.emplace_back(rootFiles, out_file_name);
}

bool ROOTFileMerger::AreJobsUnique()
{
    for (unsigned int i = 0; i < fMergeJobs.size(); i++)
    {
        for (unsigned int j = i + 1; j < fMergeJobs.size(); j++)
        {
            if (HaveDuplicates(fMergeJobs[i].in_file_names, fMergeJobs[j].in_file_names))
            {
                std::cout << "Jobs contain identical input files!" << std::endl;
                return false;
            }

            if (fMergeJobs[i].out_file_name == fMergeJobs[j].out_file_name)
            {
                std::cout << "File " << fMergeJobs[i].out_file_name << " is present more than once in the list of files to be reduced"
                          << std::endl;
                return false;
            }
        }
    }
    return true;
}

void ROOTFileMerger::SubMerge(std::vector<reducer::MergeJob>::iterator job_start, std::vector<reducer::MergeJob>::iterator job_end)
{
    for (auto job_it = job_start; job_it != job_end; ++job_it)
    {
        std::string command{"hadd -f "};
        command += job_it->out_file_name;
        std::for_each(job_it->in_file_names.begin(), job_it->in_file_names.end(),
                      [&command](const auto &file_name) { command += " " + file_name; });
        if(!system(command.c_str())) std::runtime_error("Failed to execute command: " + command + "\n");

        // std::cout << "          SumAll() haddding command: " << command << std::endl;
        // std::cout << "          Finished hadd" << std::endl;
    }
}

void ROOTFileMerger::Merge()
{
    assert(AreJobsUnique() && "Jobs are not unique");
    static const unsigned int nthreads = std::thread::hardware_concurrency();
    if(fMergeJobs.size() == 0) 
    {
        std::cout << "No jobs to merge" << std::endl;
        return;
    }
    std::cout << "Starting merging " << fMergeJobs.size() << " jobs" << std::endl;
    

    unsigned int total_forks    = fMergeJobs.size() > nthreads ? nthreads : fMergeJobs.size();
    int          activeChildren = static_cast<int>(total_forks);

    for (unsigned int nfork = 0; nfork < total_forks; ++nfork)
    {
        unsigned int start = (nfork * fMergeJobs.size() / total_forks);
        unsigned int stop  = (nfork + 1) * fMergeJobs.size() / total_forks > fMergeJobs.size()
                                ? fMergeJobs.size()
                                : (nfork + 1) * fMergeJobs.size() / total_forks;
        std::vector<MergeJob>::iterator jobs_start = fMergeJobs.begin() + start;
        std::vector<MergeJob>::iterator jobs_end   = fMergeJobs.begin() + stop;

        pid_t childPid = fork();

        if (childPid == -1) { std::runtime_error("Fork failed"); }
        if (childPid == 0)
        {
            SubMerge(jobs_start, jobs_end);
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

} // namespace reducer