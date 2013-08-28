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

    virtual void run(std::shared_ptr<Task> task) final;
    virtual void run(std::shared_ptr<Task> task, const std::chrono::high_resolution_clock::time_point& time) final;
    virtual void shutdown() final;
    virtual void waitForTasksToComplete();

    inline virtual const bool isRunning() final;

    /**
     * Retrieve the asio service that this manager is using.
     * @return Reference to boost::asio::io_service that is being used
     */
    inline std::shared_ptr<boost::asio::io_service> getService() const;
protected:
    class Tasks;

    std::shared_ptr<Tasks> mTasks;
    std::atomic_bool mRunning;
    std::shared_ptr<boost::asio::io_service> mService;
    std::unique_ptr<boost::thread_group> mThreads;
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
std::shared_ptr<boost::asio::io_service> AsioManager::getService() const
{
    return mService;
}

}
}