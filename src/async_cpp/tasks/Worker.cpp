#include "async_cpp/tasks/Worker.h"
#include "async_cpp/tasks/Task.h"

#include <assert.h>
#include <iostream>

namespace async_cpp {
namespace tasks {

//------------------------------------------------------------------------------
Worker::Worker(std::function<void (std::shared_ptr<Worker>)> askForTaskFunction, std::function<void(void)> taskCompleteFunction) 
    : mAskForTaskFunction(askForTaskFunction), mTaskCompleteFunction(taskCompleteFunction)
{
    mRunning = false;
    mThread = std::thread(&Worker::run, this);

    std::unique_lock<std::mutex> lock(mTaskMutex);
    mReadySignal.wait(lock, [this]()->bool {
        return mRunning;
    } );
}

//------------------------------------------------------------------------------
Worker::~Worker()
{  
    mRunning.exchange(false);
    {
        std::lock_guard<std::mutex> lock(mTaskMutex);
        mTaskSignal.notify_all();
    }

    if(mThread.joinable())
    {
        mThread.join();
    }

    if(mTaskToRun)
    {
        mTaskToRun->failToPerform();
        mTaskCompleteFunction();
    }
}

//------------------------------------------------------------------------------
void Worker::runTask(std::shared_ptr<Task> task)
{
    assert(task);
    assert(!mTaskToRun);

    if(mRunning.load())
    {
        {
            std::lock_guard<std::mutex> lock(mTaskMutex);
            mTaskToRun = task;
        }

        mTaskSignal.notify_all();
    }
    else
    {
        task->failToPerform();
        mTaskCompleteFunction();
    }
}

//------------------------------------------------------------------------------
void Worker::run()
{
    std::unique_lock<std::mutex> lock(mTaskMutex);
    mRunning.exchange(true);
    mReadySignal.notify_all();

    while(mRunning.load())
    {
        std::shared_ptr<Task> taskToRun;
        {
            //wait until we have a task, or we're not running
            mTaskSignal.wait(lock, [this]()->bool {
                return (nullptr != mTaskToRun) || (mRunning == false);
            } );

            taskToRun.swap(mTaskToRun);
            lock.unlock();
        }

        if(taskToRun)
        {
            if(mRunning.load())
            {
                taskToRun->perform();
            }
            else
            {
                taskToRun->failToPerform();
            }
            mTaskCompleteFunction();
            if(mRunning.load())
            {
                mAskForTaskFunction(shared_from_this());
            }

        }

        lock.lock();
    }
}

}
}
