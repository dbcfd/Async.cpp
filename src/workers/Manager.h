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

class WORKERS_API Manager {
public:
    Manager(const size_t nbWorkers);
    ~Manager();

    void run(std::shared_ptr<Task> task);
    void shutdown();
    void waitForTasksToComplete();

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