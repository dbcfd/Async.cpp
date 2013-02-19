#pragma once
#include "async/Platform.h"
#include "async/AsyncTask.h"

namespace quicktcp {
namespace async {

class ASYNC_API ParallelForEach {
public:
    ParallelForEach(std::shared_ptr<workers::Manager> manager, 
        std::function<PtrAsyncResult(std::shared_ptr<void>)> op, 
        const std::vector<std::shared_ptr<void>>& data);

    AsyncFuture execute(std::function<PtrAsyncResult(const std::vector<PtrAsyncResult>&)> onFinishTask);
    AsyncFuture execute();

private:
    std::vector<std::shared_ptr<AsyncTask>> mTasks;
    std::shared_ptr<workers::Manager> mManager;
    std::vector<std::shared_ptr<void>> mData;
};

//inline implementations
//------------------------------------------------------------------------------

}
}