#pragma once
#include "workers/Platform.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <thread>

namespace quicktcp {
namespace workers {

class Task;

class WORKERS_API Worker {
public:
    Worker(std::function<void (Worker*)> taskCompleteFunction);
    virtual ~Worker();

    void runTask(std::shared_ptr<Task> task);
    void shutdown();

    inline void waitUntilReady();
    inline const bool isRunning();
private:
    void run();

    std::unique_ptr<std::thread> mThread;
    std::promise<bool> mReadyForWorkPromise;
    std::future<bool> mReadyForWorkFuture;
    std::mutex mTaskMutex;
    std::shared_ptr<Task> mTaskToRun;
    std::condition_variable mTaskSignal;
    std::function<void (Worker*)> mTaskCompleteFunction;
    std::atomic<bool> mRunning;
};

//inline implementations
//------------------------------------------------------------------------------
void Worker::waitUntilReady()
{
     mReadyForWorkFuture.wait();
}

//------------------------------------------------------------------------------
const bool Worker::isRunning()
{
    return mRunning;
}

}
}