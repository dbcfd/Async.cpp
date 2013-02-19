#include "workers/Task.h"

namespace quicktcp {
namespace workers {

//------------------------------------------------------------------------------
Task::Task()
{

}

//------------------------------------------------------------------------------
Task::~Task()
{
    try {
        //attempt to fail the task, which sets the future to false
        failToPerform();
    }
    catch(std::future_error&)
    {
        //task was successfully performed
    }
}

//------------------------------------------------------------------------------
void Task::failToPerform()
{
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

    mTaskCompletePromise.set_value(successful);
    completeFunction();
}

}
}
