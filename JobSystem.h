#include <thread>
#include <mutex>
#include <deque>
#include <vector>
#include <atomic>
#include "Function.h"

class JobSystem
{
public:
    /// --------------------------------------------------------
    /// Constructors & Destructor
    /// --------------------------------------------------------

    /// @brief Construct a new Job System object
    /// @param numThreads Number of threads to create
    JobSystem(const uint32_t numThreads)
    {
        mThreads.reserve(numThreads);
        mRunning = true;
        for (uint32_t i = 0; i < numThreads; ++i)
        {
            mThreads.emplace_back([this]() {
                while (mRunning)
                {
                    JobData jobData;
                    {
                        std::lock_guard<std::mutex> lock(mMutex);
                        if (mJobs.empty())
                        {
                            continue;
                        }

                        mNumPendingJobs++;
                        jobData = mJobs.front();
                        mJobs.pop_front();
                    }

                    jobData.Func(jobData.Data);
                    mNumPendingJobs--;
                }
            });
        }
    }

    /// @brief Destroy the Job System object
    ~JobSystem()
    {
        mRunning = false;
        for (auto& thread : mThreads) { thread.join(); }
    }

    /// --------------------------------------------------------
    /// Getters & Setters
    /// --------------------------------------------------------

    /// @brief Get the number of threads in the job system
    /// @return Number of threads in the job system
    uint32_t GetNumThreads() const { return mThreads.size(); }

    /// @brief Get the number of pending jobs in the job system
    /// @return Number of pending jobs in the job system
    uint32_t GetNumPendingJobs() const { return mNumPendingJobs; }

    /// --------------------------------------------------------
    /// Job Management
    /// --------------------------------------------------------

    /// @brief Add a job to the job system
    /// @param function Function to execute
    /// @param data Data to pass to the function
    void AddJob(const Function<void(void*)>& function, void* data = nullptr)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mJobs.emplace_back(JobData {function, data});
    }

    /// @brief Wait for all jobs to finish
    void Wait()
    {
        while (mNumPendingJobs > 0) { continue; }
    }

private:
    struct JobData
    {
        Function<void(void*)> Func;
        void* Data;
    };

private:
    /// @brief Mutex for the job system
    std::mutex mMutex;

    /// @brief Thread function for the job system
    std::deque<JobData> mJobs;

    /// @brief Pending jobs in the job system
    std::atomic<uint32_t> mNumPendingJobs;

    /// @brief Threads in the job system
    std::vector<std::thread> mThreads;

    /// @brief Running state of the job system
    std::atomic<bool> mRunning;
};