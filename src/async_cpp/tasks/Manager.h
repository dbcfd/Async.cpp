#pragma once
#include "workers/Platform.h"
#include "workers/IManager.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

namespace async_cpp {
namespace workers {

class Worker;

/**
 * Manager of a set of workers, which are used to run tasks. If no workers are available, tasks are queue'd.
 */
class WORKERS_API Manager : public IManager {
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
     * Size indicating best possible chunk size for a large set of threads.
     * @return Best possible size for thread chunk
     */
    virtual const size_t chunkSize();

    /**
     * Flag indicating whether this manager is running. If not running, any tasks passed to it will be marked as failing to complete.
     * @return True if manager is running
     */
    inline virtual const bool isRunning();
protected:
    void run();

    size_t mNbWorkers;
    std::queue< Worker* > mWorkers;

    std::mutex mMutex;
    std::condition_variable mWorkerFinishedSignal;
    std::condition_variable mShutdownSignal;

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