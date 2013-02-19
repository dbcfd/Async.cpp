#pragma once
#include "async/Platform.h"
#include "async/AsyncTask.h"

namespace async_cpp {

namespace workers {
class IManager;
}

namespace async {

/**
 * Run a set of tasks in series using a manager, optionally executing a task when the set of parallel tasks is complete.
 * A future is created which indicates when this set of operations (including optional completion task) is completed.
 */
class ASYNC_API Series {
public:
    /**
     * Create a series task set using a manager and a set of tasks.
     * @param manager Manager to run tasks against
     * @param tasks Vector of tasks that will be run
     */
    Series(std::shared_ptr<workers::IManager> manager, const std::vector<std::function<PtrAsyncResult(PtrAsyncResult)>>& tasks);
    /**
     * Create a series task set using a manager and a set of tasks.
     * @param manager Manager to run tasks against
     * @param tasks Array of tasks that will be run
     * @param nbTasks Number of tasks in array
     */
    Series(std::shared_ptr<workers::IManager> manager, std::function<PtrAsyncResult(PtrAsyncResult)>* tasks, const size_t nbTasks);

    /**
     * Run the set of tasks in series, calling a task when the series tasks have completed.
     * @param onFinishTask Task to run when series tasks are complete
     * @return Future indicating when all operations (including onFinishTask) are complete
     */
    AsyncFuture execute(std::function<PtrAsyncResult(PtrAsyncResult)> onFinishTask);

    /**
     * Run the set of tasks in series
     * @return Future indicating when all operations are complete
     */
    AsyncFuture execute();

private:
    void addTask(std::function<PtrAsyncResult(PtrAsyncResult)> func);
    void addTask(AsyncFuture forwardedFuture, std::function<PtrAsyncResult(PtrAsyncResult)> func);

    std::vector<std::shared_ptr<IAsyncTask>> mTasks;
    std::shared_ptr<workers::IManager> mManager;
};

//inline implementations
//------------------------------------------------------------------------------

}
}