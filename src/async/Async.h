#pragma once

#include <future>
#include <memory>
#include <vector>

namespace async_cpp {
namespace async {

class AsyncResult;
class AsyncTaskResult;
class IAsyncTask;

//have to pass results as shared_ptr since std::bind and lambda is not move or forward aware
typedef std::shared_ptr<AsyncTaskResult> PtrAsyncTaskResult;
typedef std::future<AsyncResult> AsyncFuture;
typedef std::shared_ptr<std::vector<std::shared_ptr<IAsyncTask>>> ParallelTasks;

}
}