#pragma once
#include "async/Platform.h"
#include "async/AsyncTask.h"

namespace async_cpp {

namespace workers {
class IManager;
}

namespace async {

/**
 * Run a set of tasks in parallel using a manager, optionally executing a task when the set of parallel tasks is complete.
 * A future is created which indicates when this set of operations (including optional completion task) is completed.
 */
class ASYNC_API Parallel {
public:
    /**
     * Create a parallel task set using a manager and a set of tasks.
     * @param manager Manager to run tasks against
     * @param tasks Vector of tasks that will be run
     */
    Parallel(std::shared_ptr<workers::IManager> manager, const std::vector<std::function<AsyncResult(void)>>& tasks);
    /**
     * Create a parallel task set using a manager and a set of tasks.
     * @param manager Manager to run tasks against
     * @param tasks Array of tasks that will be run
     * @param nbTasks Number of tasks in array
     */
    Parallel(std::shared_ptr<workers::IManager> manager, std::function<AsyncResult(void)> tasks[], const size_t nbTasks);

    /**
     * Run the set of tasks in parallel, calling a task when the parallel tasks have completed.
     * @param onFinishTask Task to run when parallel tasks are complete
     * @return Future indicating when all operations (including onFinishTask) are complete
     */
    AsyncFuture execute(std::function<AsyncResult(AsyncResult&)> onFinishTask);

    /**
     * Run the set of tasks in parallel
     * @return Future indicating when all operations are complete
     */
    AsyncFuture execute();

private:
    std::vector<std::function<AsyncResult(void)>> mOps;
    std::shared_ptr<workers::IManager> mManager;
};

//inline implementations
//------------------------------------------------------------------------------

}
}