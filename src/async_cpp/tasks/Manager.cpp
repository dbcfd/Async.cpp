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
        //grab the next task if available, otherwise add our worker to a wait list
        std::shared_ptr<Task> task;
        if(isRunning())
        {
            {
                std::unique_lock<std::mutex> lock(mMutex);

                if(mTasks.empty())
                {
                    mWorkers.push(worker);
                }
                else
                {
                    //task available, run it
                    task.swap(mTasks.front());
                    mTasks.pop();
                }
            }
        }
        else
        {
            std::unique_lock<std::mutex> lock(mMutex);
            mWorkers.push(worker);
        }
        if(nullptr != task)
        {
            worker->runTask(task);
        }
        else
        {   
            mWorkerFinishedSignal.notify_one();
        }
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
        std::queue< std::shared_ptr<Task> > tasks;
        {
            std::unique_lock<std::mutex> lock(mMutex);

            //clear out tasks
            {
                std::queue< std::shared_ptr<Task> > empty;
                std::swap(empty, mTasks);
            }

            //wait for all running tasks to finish
            mWorkerFinishedSignal.wait(lock, [this]()->bool {
                return mWorkers.size() == mNbWorkers;
            } );
        }

        while(!tasks.empty())
        {
            run(tasks.front());
            tasks.pop();
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
        return (mWorkers.size() == mNbWorkers) && mTasks.empty();
    } );
}

//------------------------------------------------------------------------------
void Manager::run(std::shared_ptr<Task> task)
{
    assert(nullptr != task);

    //we want to run this task in a worker if one is available, else, add it to a queue
    if(isRunning())
    {
        Worker* worker = nullptr;
        {
            std::unique_lock<std::mutex> lock(mMutex);
            if(mWorkers.empty())
            {
                //no workers available, queue the task
                mTasks.push(std::move(task));
            }
            else
            {
                //worker available, grab it, and run task
                worker = std::move(mWorkers.front());
                mWorkers.pop();
            }
        }
        if(nullptr != worker)
        {
            worker->runTask(task);
        }
    }
    else
    {
        task->failToPerform();
    }
}

//------------------------------------------------------------------------------
const size_t Manager::chunkSize()
{
    size_t chunk = 1;
    {
        std::unique_lock<std::mutex> lock(mMutex);
        chunk = std::max(size_t(1), size_t(3 * mWorkers.size() / 4));
    }
    return chunk;

}

}
}
