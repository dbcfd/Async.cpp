#ifdef HAS_BOOST
#include "async_cpp/tasks/AsioManager.h"
#include "async_cpp/tasks/Task.h"

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>

#include <assert.h>

namespace async_cpp {
namespace tasks {

//------------------------------------------------------------------------------
class AsioManager::WorkWrapper : boost::asio::io_service::work {
public:
    WorkWrapper(boost::asio::io_service& service) : boost::asio::io_service::work(service)
    {

    }
};

//------------------------------------------------------------------------------
AsioManager::AsioManager(const size_t nbThreads, std::shared_ptr<boost::asio::io_service> service)
    : IManager(), mService(service), mNbThreads(nbThreads)
{
    if(!mService)
    {
        mService = std::make_shared<boost::asio::io_service>();
    }
    mRunning.store(true);
    mTasksOutstanding.store(0);
    mWork = std::unique_ptr<WorkWrapper>(new WorkWrapper(*mService));
    mThreads = std::unique_ptr<boost::thread_group>(new boost::thread_group());
    
    for(size_t i = 0; i < nbThreads; ++i)
    {
        mThreads->create_thread([this]()->void {
            mService->run();
        });
    }
}

//------------------------------------------------------------------------------
AsioManager::~AsioManager()
{
    shutdown();
    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
    mShutdownSignal.wait(lock, [this]()->bool {
        return (nullptr == mThreads);
    } );
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
        mService->stop();
        mThreads->interrupt_all();
        while(!mTasksPending.empty())
        {
            mTasksPending.front()->failToPerform();
            mTasksPending.pop();
        }
        mThreads->join_all();
        mThreads.reset();
        mShutdownSignal.notify_all();
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
        mService->post([this]()->void {
            if(mRunning)
            {
                std::shared_ptr<Task> task;
                {
                    std::unique_lock<std::mutex> lock(mTasksMutex);
                    task = mTasksPending.front();
                    mTasksPending.pop();
                }
                mTasksOutstanding.fetch_add(1);
                try
                {
                    task->perform();
                    mTasksOutstanding.fetch_sub(1);
                    mTasksSignal.notify_all();
                }
                catch(...)
                {
                    mTasksOutstanding.fetch_sub(1);
                    mTasksSignal.notify_all();
                }
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
