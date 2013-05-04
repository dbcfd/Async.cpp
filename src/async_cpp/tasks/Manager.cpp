#include "async_cpp/tasks/Manager.h"
#include "async_cpp/tasks/Worker.h"
#include "async_cpp/tasks/Task.h"

#include <functional>

#include <assert.h>

namespace async_cpp {
namespace tasks {

//------------------------------------------------------------------------------
Manager::Manager(const size_t nbWorkers) : IManager()
{
    assert(nbWorkers > 0);
    mRunning = true;
    mTasksOutstanding = 0;

    mTaskCompleteFunction = [this]() -> void {
        mTasksOutstanding.fetch_sub(1);
        mTaskFinishedSignal.notify_all();
    };


    mWorkerDoneFunction = [this](std::shared_ptr<Worker> worker) -> void {
        //grab the next task if available, otherwise add our worker to a wait list
        std::shared_ptr<Task> task;
        if(mRunning)
        {
            std::unique_lock<std::mutex> lock(mMutex);

            if(mTasks.empty())
            {
                mAvailableWorkers.push(worker);
            }
            else
            {
                //task available, run it
                task = mTasks.front();
                mTasks.pop();
            }
        }
        if(task)
        {
            mTasksOutstanding.fetch_add(1);
            worker->runTask(task);
        }
    };

    mWorkers.reserve(nbWorkers);
    for(size_t workerIdx = 0; workerIdx < nbWorkers; ++workerIdx)
    {
        auto worker = std::make_shared<Worker>(std::cref(mWorkerDoneFunction), std::cref(mTaskCompleteFunction));
        mWorkers.emplace_back(worker); //use workers to guarantee workers aren't deleted until we want them to be
        mAvailableWorkers.push(worker);
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
            mTaskFinishedSignal.wait(lock, [this]()->bool {
                return (0 == mTasksOutstanding.load());
            } );
        }

        while(!tasks.empty())
        {
            tasks.front()->failToPerform();
            tasks.pop();
        }

        while(!mAvailableWorkers.empty())
        {
            mAvailableWorkers.pop();
        }
        mWorkers.clear(); //everything is finished, now we can clean up our workers
        
        mShutdownSignal.notify_all();
    }
}

//------------------------------------------------------------------------------
void Manager::waitForTasksToComplete()
{
    std::unique_lock<std::mutex> lock(mMutex);

    mTaskFinishedSignal.wait(lock, [this]()->bool {
        return (0 == mTasksOutstanding.load() && mTasks.empty());
    } );
}

//------------------------------------------------------------------------------
void Manager::run(std::shared_ptr<Task> task)
{
    assert(nullptr != task);

    //we want to run this task in a worker if one is available, else, add it to a queue
    if(isRunning())
    {
        std::shared_ptr<Worker> worker;
        {
            std::unique_lock<std::mutex> lock(mMutex);
            if(mAvailableWorkers.empty())
            {
                //no workers available, queue the task
                mTasks.push(std::move(task));
            }
            else
            {
                //worker available, grab it, and run task
                worker = mAvailableWorkers.front();
                mAvailableWorkers.pop();
            }
        }
        if(worker)
        {
            mTasksOutstanding.fetch_add(1);
            worker->runTask(task);
        }
    }
    else
    {
        task->failToPerform();
    }
}

}
}
