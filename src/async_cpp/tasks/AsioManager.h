#pragma once
#include "async_cpp/tasks/Tasks.h"
#include "async_cpp/tasks/IManager.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

namespace boost {
namespace asio {
class io_service;
}
class thread_group;
}

namespace async_cpp {
namespace tasks {

/**
 * Manager of a set of workers, which are used to run tasks. If no workers are available, tasks are queue'd.
 */
class ASYNC_CPP_TASKS_API AsioManager : public IManager {
public:
    /**
     * Create a manager with a set number of threads, which will run tasks as they become available.
     * @param nbThreads Threads to use with service
     * @param ioService Shared pointer to boost::asio::io_service to use for thread management
     */
    AsioManager(const size_t nbThreads, std::shared_ptr<boost::asio::io_service> service = std::shared_ptr<boost::asio::io_service>());
    ~AsioManager();

    /**
     * Run a task on the first available worker, queueing if none are available.
     * @param task Task to run
     */
    virtual void run(std::shared_ptr<Task> task) final;

    /**
     * Run a task in this manager at a specified time. If manager is shutdown, task will fail to perform.
     * @param task Task to run
     */
    virtual void run(std::shared_ptr<Task> task, const std::chrono::high_resolution_clock::time_point& time) final;

    /**
     * Shutdown this manager, including all workers. Any queued tasks will be marked as failing to complete.
     */
    virtual void shutdown() final;

    /**
     * Wait for all tasks running or queued to complete.
     */
    virtual void waitForTasksToComplete() final;

    /**
     * Flag indicating whether this manager is running. If not running, any tasks passed to it will be marked as failing to complete.
     * @return True if manager is running
     */
    inline virtual const bool isRunning() final;

    /**
     * The ideal number of tasks that can be run at once, such that they should all run at approximately the same time. This is
     * most likely the number of threads/workers.
     * @return Ideal number of tasks to run at once
     */
    inline virtual size_t idealNumberOfSimultaneousTasks() const final;

    /**
     * Retrieve the asio service that this manager is using.
     * @return Reference to boost::asio::io_service that is being used
     */
    inline std::shared_ptr<boost::asio::io_service> getService();
protected:
    void runNextTask(std::shared_ptr<IManager> manager);

    std::atomic_bool mRunning;
    std::atomic_size_t mTasksOutstanding;
    std::mutex mTasksMutex;
    std::condition_variable mTasksSignal;
    std::queue<std::shared_ptr<Task>> mTasksPending;
    std::shared_ptr<boost::asio::io_service> mService;
    std::unique_ptr<boost::thread_group> mThreads;
    std::condition_variable mShutdownSignal;
    bool mCreatedService;
    size_t mNbThreads;
};

//inline implementations
//------------------------------------------------------------------------------
const bool AsioManager::isRunning()
{
    return mRunning;
}

//------------------------------------------------------------------------------
std::shared_ptr<boost::asio::io_service> AsioManager::getService()
{
    return mService;
}

//------------------------------------------------------------------------------
size_t AsioManager::idealNumberOfSimultaneousTasks() const
{
    return mNbThreads;
}

}
}