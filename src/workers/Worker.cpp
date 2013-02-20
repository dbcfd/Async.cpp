#include "workers/Worker.h"
#include "workers/Task.h"

namespace async_cpp {
namespace workers {

//------------------------------------------------------------------------------
Worker::Worker(std::function<void (Worker*)> taskCompleteFunction) : mRunning(true), mTaskCompleteFunction(taskCompleteFunction)
{
    mReadyForWorkFuture = mReadyForWorkPromise.get_future();
    mThread = std::unique_ptr<std::thread>(new std::thread(&Worker::run, this));
}

//------------------------------------------------------------------------------
Worker::~Worker()
{
    shutdown();
}

//------------------------------------------------------------------------------
void Worker::shutdown()
{
    bool wasRunning = mRunning.exchange(false);
    
    if(wasRunning)
    {
        mTaskSignal.notify_all();
        mThread->join();
    }
}

//------------------------------------------------------------------------------
void Worker::runTask(std::shared_ptr<Task> task)
{
    if(isRunning())
    {
        {
            std::unique_lock<std::mutex> lock(mTaskMutex);
            mTaskToRun = task;
        }

        mTaskSignal.notify_all();
    }
    else if(nullptr != task)
    {
        task->failToPerform();
        mTaskCompleteFunction(this);
    }
}

//------------------------------------------------------------------------------
void Worker::run()
{
    mReadyForWorkPromise.set_value(true);

    while(isRunning())
    {
        std::shared_ptr<Task> taskToRun;
        {
            std::unique_lock<std::mutex> lock(mTaskMutex);

            mTaskSignal.wait(lock, [this]()->bool {
                return (nullptr != mTaskToRun) || !isRunning();
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
