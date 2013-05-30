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
                catch(std::exception& ex)
                {
                    onException(ex);
                }
            }
            return false;
        } );
    mTaskCompleteFuture = mTask.get_future();
}

//------------------------------------------------------------------------------
Task::~Task()
{
    failToPerform();
}

//------------------------------------------------------------------------------
void Task::onException(const std::exception&)
{
    //do nothing
}

//------------------------------------------------------------------------------
void Task::notifyFailureToPerform()
{
    //do nothing
}

//------------------------------------------------------------------------------
void Task::failToPerform()
{
    bool wasInvoked = mTaskInvoked.exchange(true);
    if(!wasInvoked) 
    {
        mTask(true);
        this->notifyFailureToPerform();
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
