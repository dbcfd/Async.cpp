#include "workers/Task.h"

#include <iostream>

namespace async_cpp {
namespace workers {

//------------------------------------------------------------------------------
Task::Task()
{
    mTask = std::packaged_task<bool(bool, std::function<void(void)>)>(
        [this](bool isFailing, std::function<void(void)> completeFunction)-> bool 
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
                    std::cout << "Task: Error when running, " << ex.what() << std::endl;
                }
    
                completeFunction();
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
void Task::failToPerform()
{
    mTask(true, [](){});
}

//------------------------------------------------------------------------------
void Task::reset()
{
    mTask.reset();
    mTaskCompleteFuture = mTask.get_future();
}

//------------------------------------------------------------------------------
void Task::perform(std::function<void(void)> completeFunction)
{
    mTask(false, completeFunction);
}

}
}
