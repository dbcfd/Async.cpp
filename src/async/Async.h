#pragma once

#include <future>
#include <memory>

namespace async_cpp {
namespace async {

class AsyncResult;

//have to pass results as shared_ptr since std::bind and lambda is not move or forward aware
typedef std::shared_ptr<AsyncResult> PtrAsyncResult;
typedef std::future<PtrAsyncResult> AsyncFuture;

}
}