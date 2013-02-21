#include "async/AsyncTask.h"
#include "async/AsyncResult.h"

namespace async_cpp {
namespace async {

//------------------------------------------------------------------------------
AsyncTask::AsyncTask(std::function<void(void)> generateResult)
    : mGenerateResultFunc(std::move(generateResult))
{

}

//------------------------------------------------------------------------------
void AsyncTask::performSpecific()
{
    mGenerateResultFunc();
}

//------------------------------------------------------------------------------
IForwardingTask::IForwardingTask()
{

}

//------------------------------------------------------------------------------
IForwardingTask::~IForwardingTask()
{

}

//------------------------------------------------------------------------------
AsyncForwardTask::AsyncForwardTask(std::function<void(AsyncResult&)> generateResult)
    : mGenerateResultFunc(std::move(generateResult))
{

}

//------------------------------------------------------------------------------
void AsyncForwardTask::performSpecific()
{
    mGenerateResultFunc(mForwardedResult);
}

//------------------------------------------------------------------------------
AsyncTerminalTask::AsyncTerminalTask(std::function<AsyncResult(AsyncResult&)> generateResult) : mGenerateResultFunc(std::move(generateResult))
{

}

//------------------------------------------------------------------------------
void AsyncTerminalTask::performSpecific()
{
    mPromise.set_value(mGenerateResultFunc(std::move(mForwardedResult)));
}

}
}
