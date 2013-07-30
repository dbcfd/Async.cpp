#include "async_cpp/tasks/Task.h"

#include <iostream>

namespace async_cpp {
namespace tasks {

//------------------------------------------------------------------------------
Task::Task()
{
    mTaskInvoked = false;
    buildMembers();
}

//------------------------------------------------------------------------------
void Task::buildMembers()
{
    mTask = std::packaged_task<bool(bool)>(
        [this](bool isFailing)-> bool 
        {
            if(!isFailing)
            {
                try 
                {
                    performSpecific();
                    return true;
                }
                catch(...)
                {
                    try
                    {
                        notifyException(std::current_exception());
                    }
                    catch(...)
                    {
                        //notify caused an exception
                    }
                }
            }
            return false;
        } );
    mTaskCompleteFuture = mTask.get_future();
}

//------------------------------------------------------------------------------
Task::~Task()
{
    cancel();
}

//------------------------------------------------------------------------------
void Task::notifyException(std::exception_ptr ex)
{
    //do nothing
}

//------------------------------------------------------------------------------
void Task::notifyCancel()
{
    //do nothing
}

//------------------------------------------------------------------------------
void Task::cancel()
{
    bool wasInvoked = mTaskInvoked.exchange(true);
    if(!wasInvoked) 
    {
        mTask(true);
        this->notifyCancel();
    }
}

//------------------------------------------------------------------------------
void Task::perform()
{
    bool wasInvoked = mTaskInvoked.exchange(true);
    if(!wasInvoked) mTask(false);
}

}
}
