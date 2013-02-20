#include "workers/Worker.h"
#include "workers/Task.h"

#include <assert.h>

namespace async_cpp {
namespace workers {

//------------------------------------------------------------------------------
Worker::Worker(std::function<void (Worker*)> taskCompleteFunction) : mRunning(true), mTaskCompleteFunction(taskCompleteFunction)
{
    mThread = std::unique_ptr<std::thread>(new std::thread(&Worker::run, this));
}

//------------------------------------------------------------------------------
Worker::~Worker()
{
    shutdown();
    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
    mShutdownSignal.wait(lock, [this]()->bool {
        return (nullptr == mThread);
    } );
}

//------------------------------------------------------------------------------
void Worker::shutdown()
{
    bool wasRunning = mRunning.exchange(false);
    
    if(wasRunning)
    {
        mTaskSignal.notify_all();
        mThread->join();
        if(nullptr != mTaskToRun)
        {
            mTaskToRun->failToPerform();
            mTaskCompleteFunction(this);
        }
        mThread = nullptr;
        mShutdownSignal.notify_one();
    }
}

//------------------------------------------------------------------------------
void Worker::runTask(std::shared_ptr<Task> task)
{
    assert(nullptr != task);

    if(mRunning)
    {
        {
            std::unique_lock<std::mutex> lock(mTaskMutex);
            mTaskToRun = task;
        }

        mTaskSignal.notify_all();
    }
    else
    {
        task->failToPerform();
        mTaskCompleteFunction(this);
    }
}

//------------------------------------------------------------------------------
void Worker::run()
{
    while(mRunning)
    {
        std::shared_ptr<Task> taskToRun;
        {
            std::unique_lock<std::mutex> lock(mTaskMutex);

            //wait until we have a task, or we're not running
            mTaskSignal.wait(lock, [this]()->bool {
                return (nullptr != mTaskToRun) || (mRunning == false);
            } );

            taskToRun.swap(mTaskToRun);
        }

        if(nullptr != taskToRun)
        {
            taskToRun->perform([](){});
            mTaskCompleteFunction(this);
        }
    }
}

}
}
