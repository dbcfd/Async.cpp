#include "workers/BasicTask.h"

namespace async_cpp {
namespace workers {

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
