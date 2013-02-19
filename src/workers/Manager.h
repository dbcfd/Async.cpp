#pragma once
#include "workers/Platform.h"

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

namespace quicktcp {
namespace workers {

class Task;
class Worker;

/**
 * Manager of a set of workers, which are used to run tasks.
 */
class WORKERS_API Manager {
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
    void run(std::shared_ptr<Task> task);

    /**
     * Shutdown this manager, including all workers. Any queued tasks will be marked as failing to complete.
     */
    void shutdown();

    /**
     * Wait for all tasks running or queued to complete.
     */
    void waitForTasksToComplete();

    /**
     * Flag indicating whether this manager is running. If not running, any tasks passed to it will be marked as failing to complete.
     * @return True if manager is running
     */
    inline const bool isRunning();
protected:
    void run();

    size_t mNbWorkers;
    std::queue< Worker* > mWorkers;

    std::mutex mMutex;
    std::condition_variable mWorkerFinishedSignal;

    std::queue< std::shared_ptr<Task> > mTasks;

    std::atomic<bool> mRunning;
};

//inline implementations
//------------------------------------------------------------------------------
const bool Manager::isRunning()
{
    return mRunning;
}

}
}