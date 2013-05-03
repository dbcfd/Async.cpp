#include "async_cpp/tasks/BasicTask.h"

namespace async_cpp {
namespace tasks {

//------------------------------------------------------------------------------
BasicTask::BasicTask(std::function<void(void)> functionToRun) : mFunctionToRun(functionToRun)
{

}

//------------------------------------------------------------------------------
BasicTask::~BasicTask()
{

}

//------------------------------------------------------------------------------
void BasicTask::performSpecific()
{
    mFunctionToRun();
}

}
}
