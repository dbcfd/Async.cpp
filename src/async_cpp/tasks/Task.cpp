#include "async_cpp/tasks/Task.h"

#include <iostream>

namespace async_cpp {
namespace tasks {

//------------------------------------------------------------------------------
Task::Task()
{
    mTask = std::packaged_task<bool(bool)>(
        [this](bool isFailing)-> bool 
        {
            bool successful = false;
            if(!isFailing)
            {
                try 
                {
                    performSpecific();
                    successful = true;
                }
                catch(std::exception& ex)
                {
                    onException(ex);
                }
            }
            return successful;
        } );
    mTaskCompleteFuture = mTask.get_future();
}

//------------------------------------------------------------------------------
Task::~Task()
{
    try
    {
        if(mTaskCompleteFuture.valid())
        {
            failToPerform();
        }
    }
    catch(std::future_error&)
    {
        //already satisfied
    }
}

//------------------------------------------------------------------------------
void Task::onException(const std::exception&)
{
    //do nothing
}

//------------------------------------------------------------------------------
void Task::failToPerform()
{
    mTask(true);
}

//------------------------------------------------------------------------------
void Task::reset()
{
    mTask.reset();
    mTaskCompleteFuture = mTask.get_future();
}

//------------------------------------------------------------------------------
void Task::perform()
{
    mTask(false);
}

}
}
