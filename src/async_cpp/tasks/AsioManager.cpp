#include "async_cpp/tasks/AsioManager.h"
#include "async_cpp/tasks/Task.h"

#include <boost/thread/thread.hpp>

namespace async_cpp {
namespace tasks {

//------------------------------------------------------------------------------
class AsioManager::Tasks {
public:
    Tasks()
    {

    }

    std::shared_ptr<Task> get()
    {
        std::shared_ptr<Task> task;
        {
            std::lock_guard<std::mutex> lock(mMutex);
            if(!mTasks.empty())
            {
                task = mTasks.front();
                mTasks.pop();
            }
        }
        return task;
    }

    void add(std::shared_ptr<Task> task)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mTasks.push(task);
    }

    void cancel()
    {
        std::lock_guard<std::mutex> lock(mMutex);
        while(!mTasks.empty())
        {
            mTasks.front()->cancel();
            mTasks.pop();
        }
    }

    void notifyCompletion()
    {
        mTaskCompleteSignal.notify_all();
    }

    void waitForTasksToComplete()
    {
        std::unique_lock<std::mutex> lock(mMutex);
        mTaskCompleteSignal.wait(lock, [this]()->bool 
        {
            return mTasks.empty();
        } );
    }

private:
    std::mutex mMutex;
    std::queue<std::shared_ptr<Task>> mTasks;
    std::condition_variable mTaskCompleteSignal;
};

//------------------------------------------------------------------------------
AsioManager::AsioManager(const size_t nbThreads, std::shared_ptr<boost::asio::io_service> service)
    : IManager(), mNbThreads(nbThreads), mCreatedService(false), mTasks(std::make_shared<Tasks>())
{
    if(!service)
    {
        service = std::make_shared<boost::asio::io_service>();
        mCreatedService = true;
    }
    mService = service;
    mRunning.store(true);
    mWork = std::make_shared<boost::asio::io_service::work>(*mService);
    mThreads = std::unique_ptr<boost::thread_group>(new boost::thread_group());
    
    auto tasks = mTasks;
    for(size_t i = 0; i < nbThreads; ++i)
    {
        mThreads->create_thread([service]()->void {
            service->run();
        });
    }
}

//------------------------------------------------------------------------------
AsioManager::~AsioManager()
{
    shutdown();
}

//------------------------------------------------------------------------------
void AsioManager::shutdown()
{
    bool wasRunning = mRunning.exchange(false);
    if(wasRunning)
    {
        //free work so service can stop
        mWork.reset();

        //stop service if we created it
        if(mCreatedService)
        {
            mService->stop();
        }

        //cancel the remaining tasks that aren't running
        mTasks->cancel();
        mTasks->waitForTasksToComplete();
        mTasks.reset();

        //stop all the threads
        mThreads->interrupt_all();
        mThreads->join_all();
        mThreads.reset();
    }
}

//------------------------------------------------------------------------------
void AsioManager::waitForTasksToComplete()
{
    mTasks->waitForTasksToComplete();
}

//------------------------------------------------------------------------------
void AsioManager::run(std::shared_ptr<Task> task)
{
    if(task)
    {
        if(mRunning.load())
        {
            mTasks->add(task);
            auto tasks = mTasks;
            mService->post([tasks]()->void
            {
                auto task = tasks->get();
                if(task)
                {
                    task->perform();
                }
                tasks->notifyCompletion();
            } );
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
                std::weak_ptr<IManager> managerPtr = shared_from_this();
                auto taskRunTimer = std::make_shared<boost::asio::deadline_timer>(*mService, boost::posix_time::microseconds((long)dur.count()));
                taskRunTimer->async_wait([task, taskRunTimer, managerPtr](const boost::system::error_code& ec)->void 
                {
                    if(!ec)
                    {
                        auto manager = managerPtr.lock();
                        if(manager)
                        {
                            manager->run(task);
                        }
                        else
                        {
                            task->cancel();
                        }
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
