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
        mService->stop();
        {
            std::unique_lock<std::mutex> lock(mTasksMutex);
            mTasksSignal.wait(lock, [this]()->bool {
                return (0 == mTasksOutstanding.load());
            } );
            while(!mTasksPending.empty())
            {
                mTasksPending.front()->cancel();
                mTasksPending.pop();
            }
        }
        mThreads->interrupt_all();
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
void AsioManager::runNextTask(std::shared_ptr<IManager> manager)
{
    if(mRunning.load())
    {
        std::shared_ptr<Task> task;
        {
            std::lock_guard<std::mutex> lock(mTasksMutex);
            if(!mTasksPending.empty())
            {
                task = mTasksPending.front();
                mTasksPending.pop();
            }
        }
        if(task)
        {
            mTasksOutstanding.fetch_add(1);
            task->perform();
            mTasksOutstanding.fetch_sub(1);
            std::lock_guard<std::mutex> lock(mTasksMutex);
            mTasksSignal.notify_all();
        }
    }
}

//------------------------------------------------------------------------------
void AsioManager::run(std::shared_ptr<Task> task)
{
    if(task)
    {
        if(mRunning.load())
        {
            {
                std::lock_guard<std::mutex> lock(mTasksMutex);
                mTasksPending.push(task);
            }
            mService->post(std::bind(&AsioManager::runNextTask, this, shared_from_this()));
        }  
        else
        {
            task->cancel();
        }
    }
}

//------------------------------------------------------------------------------
void AsioManager::run(std::shared_ptr<Task> task, const std::chrono::high_resolution_clock::time_point& time)
{
    if(task)
    {
        if(mRunning)
        {
            auto dur = std::chrono::duration_cast<std::chrono::microseconds>(time - std::chrono::high_resolution_clock::now());
            if(dur.count() < 0)
            {
                run(task);
            }
            else
            {
                auto taskRunTimer = std::make_shared<boost::asio::deadline_timer>(*mService, boost::posix_time::microseconds((long)dur.count()));
                taskRunTimer->async_wait([task, taskRunTimer, this](const boost::system::error_code& ec)->void {
                    if(!ec)
                    {
                        run(task);
                    }
                    else
                    {
                        task->cancel();
                    }
                } );
            }
        }  
        else
        {
            task->cancel();
        }
    }
}

}
}

#endif
