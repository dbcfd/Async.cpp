#ifdef HAS_BOOST
#include "async_cpp/tasks/AsioManager.h"
#include "async_cpp/tasks/Task.h"

#include <assert.h>

namespace async_cpp {
namespace tasks {

//------------------------------------------------------------------------------
AsioManager::AsioManager(const size_t nbThreads)
    : IManager(), mWork(mService)
{
    mRunning.store(true);
    mTasksOutstanding.store(0);
    
    for(size_t i = 0; i < nbThreads; ++i)
    {
        mThreads.emplace_back([this]()->void {
            mService.run();
        });
    }
}

//------------------------------------------------------------------------------
AsioManager::~AsioManager()
{
    shutdown();
    for(auto& thread : mThreads)
    {
        if(thread.joinable())
        {
            thread.join();
        }
    }
}

//------------------------------------------------------------------------------
void AsioManager::shutdown()
{
    bool wasRunning = mRunning.exchange(false);
    if(wasRunning)
    {
        {
            std::unique_lock<std::mutex> lock(mTasksMutex);
            mTasksSignal.wait(lock, [this]()->bool {
                return (0 == mTasksOutstanding.load());
            } );
        }
        while(!mTasksPending.empty())
        {
            mTasksPending.front()->failToPerform();
            mTasksPending.pop();
        }
        mService.stop();
        for(auto& thread : mThreads)
        {
            if(thread.joinable())
            {
                thread.join();
            }
        }
    }
}

//------------------------------------------------------------------------------
void AsioManager::waitForTasksToComplete()
{
    std::unique_lock<std::mutex> lock(mTasksMutex);
    mTasksSignal.wait(lock, [this]()->bool {
        return mTasksPending.empty() && (0 == mTasksOutstanding.load());
    } );
}

//------------------------------------------------------------------------------
void AsioManager::run(std::shared_ptr<Task> task)
{
    assert(task);
    if(mRunning)
    {
        {
            std::unique_lock<std::mutex> lock(mTasksMutex);
            mTasksPending.push(task);
        }
        mService.post([this]()->void {
            std::shared_ptr<Task> task;
            {
                std::unique_lock<std::mutex> lock(mTasksMutex);
                task = mTasksPending.front();
                mTasksPending.pop();
            }
            mTasksOutstanding.fetch_add(1);
            task->perform();
            mTasksOutstanding.fetch_sub(1);
            mTasksSignal.notify_all();
        } );
    }  
    else
    {
        task->failToPerform();
    }
}

}
}

#endif
