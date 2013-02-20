#include "workers/Manager.h"
#include "workers/Worker.h"
#include "workers/Task.h"

#include <functional>

#include <assert.h>

namespace async_cpp {
namespace workers {

//------------------------------------------------------------------------------
Manager::Manager(const size_t nbWorkers) : IManager(), mRunning(true), mNbWorkers(nbWorkers)
{
    assert(nbWorkers > 0);

    auto workerDoneFunction = [this](Worker* worker) -> void {
        {
            std::unique_lock<std::mutex> lock(mMutex);
            mWorkers.push(worker);
        }
        mWorkerFinishedSignal.notify_one();
    };

    std::vector<Worker*> workers;
    workers.reserve(nbWorkers);
    for(size_t workerIdx = 0; workerIdx < nbWorkers; ++workerIdx)
    {
        mWorkers.push(new Worker(workerDoneFunction));
    }
}

//------------------------------------------------------------------------------
Manager::~Manager()
{
    shutdown();
    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
    mShutdownSignal.wait(lock, [this]()->bool {
        return mWorkers.empty();
    } );
}

//------------------------------------------------------------------------------
void Manager::shutdown()
{
    bool wasRunning = mRunning.exchange(false);

    if(wasRunning)
    {
        {
            std::unique_lock<std::mutex> lock(mMutex);

            //wait for all running tasks to finish
            mWorkerFinishedSignal.wait(lock, [this]()->bool {
                return mWorkers.size() == mNbWorkers;
            } );
        }

        while(!mWorkers.empty())
        {
            Worker* worker = mWorkers.front();
            mWorkers.pop();
            worker->shutdown();
            delete worker;
        }

        mShutdownSignal.notify_all();
    }
}

//------------------------------------------------------------------------------
void Manager::waitForTasksToComplete()
{
    std::unique_lock<std::mutex> lock(mMutex);

    mWorkerFinishedSignal.wait(lock, [this]()->bool {
        return (mWorkers.size() == mNbWorkers);
    } );
}

//------------------------------------------------------------------------------
void Manager::run(std::shared_ptr<Task> task)
{
    assert(nullptr != task);

    //we want to run this task in a worker if one is available, else, add it to a queue
    if(mRunning)
    {
        Worker* worker = nullptr;
        {
            std::unique_lock<std::mutex> lock(mMutex);

            mWorkerFinishedSignal.wait(lock, [this]()->bool {
                return !mWorkers.empty();
            } );

            //worker available, grab it, and run task
            worker = mWorkers.front();
            mWorkers.pop();
        }
        worker->runTask(task);
    }
    else
    {
        task->failToPerform();
    }
}

}
}
