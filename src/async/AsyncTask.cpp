#include "async/AsyncTask.h"
#include "async/AsyncResult.h"

namespace quicktcp {
namespace async {

//------------------------------------------------------------------------------
IAsyncTask::IAsyncTask()
{

}

//------------------------------------------------------------------------------
IAsyncTask::~IAsyncTask()
{

}

//------------------------------------------------------------------------------
AsyncTask::AsyncTask(std::packaged_task<PtrAsyncResult(void)> generateResult)
    : mGenerateResultFunc(std::move(generateResult))
{

}

//------------------------------------------------------------------------------
void AsyncTask::performSpecific()
{
    mGenerateResultFunc();
}

//------------------------------------------------------------------------------
AsyncForwardTask::AsyncForwardTask(AsyncFuture forwardedFuture, 
                                   std::packaged_task<PtrAsyncResult(PtrAsyncResult)> generateResult)
    : mForwardedFuture(std::move(forwardedFuture)), mGenerateResultFunc(std::move(generateResult))
{

}

//------------------------------------------------------------------------------
void AsyncForwardTask::performSpecific()
{
    mGenerateResultFunc(std::move(mForwardedFuture.get()));
}

}
}
