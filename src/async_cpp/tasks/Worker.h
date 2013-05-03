#pragma once
#include "async_cpp/tasks/Tasks.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <thread>

namespace async_cpp {
namespace tasks {

/**
 * Object which holds a thread to run tasks in, obtaining additional tasks via its taskCompleteFunction if available.
 */
class ASYNC_CPP_TASKS_API Worker : public std::enable_shared_from_this<Worker> {
public:
    /**
     * Create a worker (and its underlying thread) to run tasks, with a function to invoke everytime it finishes running a task
     * @param taskCompleteFunction Function object to invoke when task is finished
     */
    Worker(std::function<void (std::shared_ptr<Worker>)> taskCompleteFunction);
    virtual ~Worker();

    /**
     * Set the task for this worker to run. Worker thread will be woken up if it is waiting for a task. The task will not be run until 
     * the worker thread is able to wake up and invoke the task's perform method.
     * @param task Task to run. 
     */
    void runTask(std::shared_ptr<Task> task);

    /**
     * Whether or not this worker is running. If not running, tasks passed to it will be marked as failures.
     */
    inline const bool isRunning();
private:
    void run();

    std::thread mThread;
    std::mutex mTaskMutex;
    std::shared_ptr<Task> mTaskToRun;
    std::condition_variable mTaskSignal;
    std::function<void (std::shared_ptr<Worker>)> mTaskCompleteFunction;
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