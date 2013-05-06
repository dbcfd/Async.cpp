#ifdef HAS_BOOST
#include "async_cpp/tasks/AsioManager.h"
#include "async_cpp/tasks/Task.h"

#include <boost/asio.hpp>

#include <assert.h>

namespace async_cpp {
namespace tasks {

//------------------------------------------------------------------------------
class AsioManager::WorkWrapper {
public:
    WorkWrapper(boost::asio::io_service& service) : mWork(service)
    {

    }

private:
    boost::asio::io_service::work mWork;
};

//------------------------------------------------------------------------------
AsioManager::AsioManager(const size_t nbThreads)
    : IManager(), mService(std::make_shared<boost::asio::io_service>())
{
    mRunning.store(true);
    mTasksOutstanding.store(0);
    mWork = std::unique_ptr<WorkWrapper>(new WorkWrapper(*mService));
    
    for(size_t i = 0; i < nbThreads; ++i)
    {
        mThreads.emplace_back([this]()->void {
            mService->run();
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
        mService->stop();
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
        auto thisPtr = shared_from_this();
        mService->post([thisPtr, this]()->void {
            if(mRunning)
            {
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
            }
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
