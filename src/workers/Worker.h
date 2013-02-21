#pragma once
#include "workers/Platform.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <thread>

namespace async_cpp {
namespace workers {

class Task;

/**
 * Object which holds a thread to run tasks in, obtaining additional tasks via its taskCompleteFunction if available.
 */
class WORKERS_API Worker {
public:
    /**
     * Create a worker (and its underlying thread) to run tasks, with a function to invoke everytime it finishes running a task
     * @param taskCompleteFunction Function object to invoke when task is finished
     */
    Worker(std::function<void (Worker*)> taskCompleteFunction);
    virtual ~Worker();

    /**
     * Set the task for this worker to run. Worker thread will be woken up if it is waiting for a task. The task will not be run until 
     * the worker thread is able to wake up and invoke the task's perform method.
     * @param task Task to run. 
     */
    void runTask(std::shared_ptr<Task> task);

    /**
     * Shutdown this worker. If a task is waiting to be run, it will be marked as a failure.
     */
    void shutdown();

    /**
     * Whether or not this worker is running. If not running, tasks passed to it will be marked as failures.
     */
    inline const bool isRunning();
private:
    void run();

    std::unique_ptr<std::thread> mThread;
    std::mutex mTaskMutex;
    std::shared_ptr<Task> mTaskToRun;
    std::condition_variable mTaskSignal;
    std::condition_variable mShutdownSignal;
    std::condition_variable mWorkerReadySignal;
    std::function<void (Worker*)> mTaskCompleteFunction;
    std::atomic<bool> mRunning;
};

//inline implementations
//------------------------------------------------------------------------------
const bool Worker::isRunning()
{
    return mRunning;
}

}
}