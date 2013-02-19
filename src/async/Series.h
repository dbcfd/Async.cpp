#pragma once
#include "async/Platform.h"
#include "async/AsyncTask.h"

namespace quicktcp {
namespace async {

class ASYNC_API Series {
public:
    Series(std::shared_ptr<workers::Manager> manager, const std::vector<std::function<PtrAsyncResult(PtrAsyncResult)>>& tasks);
    Series(std::shared_ptr<workers::Manager> manager, std::function<PtrAsyncResult(PtrAsyncResult)>* tasks, const size_t nbTasks);

    AsyncFuture execute(std::function<PtrAsyncResult(PtrAsyncResult)> onFinishTask);
    AsyncFuture execute();

private:
    void addTask(std::function<PtrAsyncResult(PtrAsyncResult)> func);
    void addTask(AsyncFuture forwardedFuture, std::function<PtrAsyncResult(PtrAsyncResult)> func);

    std::vector<std::shared_ptr<IAsyncTask>> mTasks;
    std::shared_ptr<workers::Manager> mManager;
};

//inline implementations
//------------------------------------------------------------------------------

}
}