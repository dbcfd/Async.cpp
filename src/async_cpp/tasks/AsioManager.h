#pragma once
#ifdef HAS_BOOST
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
     * @param ioService Shared pointer to boost::asio::io_service to use for thread management
     * @param nbThreads Threads to use with service
     */
    AsioManager(const size_t nbThreads);
    ~AsioManager();

    /**
     * Run a task on the first available worker, queueing if none are available.
     * @param task Task to run
     */
    virtual void run(std::shared_ptr<Task> task);

    /**
     * Shutdown this manager, including all workers. Any queued tasks will be marked as failing to complete.
     */
    virtual void shutdown();

    /**
     * Wait for all tasks running or queued to complete.
     */
    virtual void waitForTasksToComplete();

    /**
     * Flag indicating whether this manager is running. If not running, any tasks passed to it will be marked as failing to complete.
     * @return True if manager is running
     */
    inline virtual const bool isRunning();

    /**
     * Retrieve the asio service that this manager is using.
     * @return Reference to boost::asio::io_service that is being used
     */
    inline std::shared_ptr<boost::asio::io_service> getService();
protected:
    class WorkWrapper;

    std::atomic_bool mRunning;
    std::atomic_size_t mTasksOutstanding;
    std::mutex mTasksMutex;
    std::condition_variable mTasksSignal;
    std::queue<std::shared_ptr<Task>> mTasksPending;
    std::shared_ptr<boost::asio::io_service> mService;
    std::unique_ptr<WorkWrapper> mWork;
    std::vector<std::thread> mThreads;
    std::condition_variable mShutdownSignal;
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

}
}

#endif