#include "async_cpp/tasks/Task.h"

namespace async_cpp {
namespace tasks {

//------------------------------------------------------------------------------
Task::Task() : mTaskCompletePromise(), mTaskCompleteFuture(mTaskCompletePromise.get_future()), mHasFulfilledPromise(false)
{

}

//------------------------------------------------------------------------------
Task::~Task()
{
    if(!mHasFulfilledPromise)
    {
        failToPerform();
    }
}

//------------------------------------------------------------------------------
void Task::failToPerform()
{
    mHasFulfilledPromise = true;
    mTaskCompletePromise.set_value(false);
}

//------------------------------------------------------------------------------
void Task::perform(std::function<void(void)> completeFunction)
{
    bool successful = false;
    try 
    {
        performSpecific();
        successful = true;
    }
    catch(std::runtime_error&)
    {
        
    }

    mHasFulfilledPromise = true;
    mTaskCompletePromise.set_value(successful);
    completeFunction();
}

}
}
