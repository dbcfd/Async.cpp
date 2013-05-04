#pragma once
#include "async_cpp/tasks/Tasks.h"
#include "async_cpp/tasks/IManager.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

namespace async_cpp {
namespace tasks {

class Worker;

/**
 * Manager of a set of workers, which are used to run tasks. If no workers are available, tasks are queue'd.
 */
class ASYNC_CPP_TASKS_API Manager : public IManager {
public:
    /**
     * Create a worker with a set number of workers, which will run tasks as they become available.
     * @param nbWorkers Number of workers to create and manager
     */
    Manager(const size_t nbWorkers);
    ~Manager();

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
protected:
    void run();

    std::vector< std::shared_ptr<Worker> > mWorkers;
    std::queue< std::shared_ptr<Worker> > mAvailableWorkers;

    std::mutex mMutex;
    std::condition_variable mTaskFinishedSignal;
    std::condition_variable mShutdownSignal;
    std::function<void(std::shared_ptr<Worker>)> mWorkerDoneFunction;
    std::function<void(void)> mTaskCompleteFunction;

    std::queue< std::shared_ptr<Task> > mTasks;

    std::atomic_bool mRunning;
    std::atomic_size_t mTasksOutstanding;
};

//inline implementations
//------------------------------------------------------------------------------
const bool Manager::isRunning()
{
    return mRunning;
}

}
}