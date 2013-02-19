#pragma once
#include "async/Platform.h"
#include "async/AsyncTask.h"

#include <stdarg.h>

namespace quicktcp {
namespace async {

class ASYNC_API Parallel {
public:
    Parallel(std::shared_ptr<workers::Manager> manager, const std::vector<std::function<PtrAsyncResult(void)>>& tasks);
    Parallel(std::shared_ptr<workers::Manager> manager, std::function<PtrAsyncResult(void)>* tasks, const size_t nbTasks);

    AsyncFuture execute(std::function<PtrAsyncResult(PtrAsyncResult)> onFinishTask);
    AsyncFuture execute();

private:
    std::vector<std::shared_ptr<IAsyncTask>> mTasks;
    std::shared_ptr<workers::Manager> mManager;
};

//inline implementations
//------------------------------------------------------------------------------

}
}