#pragma once
#include "async_cpp/async/Platform.h"
#include "async_cpp/async/Async.h"

#include <functional>

namespace async_cpp {

namespace tasks {
class IManager;
}

namespace async {

class AsyncTerminalTask;

/**
 * Run a set of tasks in series using a manager, optionally executing a task when the set of parallel tasks is complete.
 * A future is created which indicates when this set of operations (including optional completion task) is completed.
 */
class ASYNC_CPP_ASYNC_API Series {
public:
    /**
     * Create a series task set using a manager and a set of tasks.
     * @param manager Manager to run tasks against
     * @param ops Vector of tasks that will be run
     */
    Series(std::shared_ptr<tasks::IManager> manager, const std::vector<std::function<AsyncFuture(AsyncResult&)>>& ops);
    /**
     * Create a series task set using a manager and a set of tasks.
     * @param manager Manager to run tasks against
     * @param ops Array of operations that will be run
     * @param nbOps Number of operations in array
     */
    Series(std::shared_ptr<tasks::IManager> manager, std::function<AsyncFuture(AsyncResult&)>* ops, const size_t nbOps);

    /**
     * Run the set of tasks in series, calling a task when the series tasks have completed.
     * @param onFinishTask Task to run when series tasks are complete
     * @return Future indicating when all operations (including onFinishTask) are complete
     */
    AsyncFuture execute(std::function<AsyncFuture(AsyncResult&)> onFinishTask);

    /**
     * Run the set of tasks in series
     * @return Future indicating when all operations are complete
     */
    AsyncFuture execute();

private:
    std::vector<std::function<AsyncFuture(AsyncResult&)>> mOperations;
    std::shared_ptr<tasks::IManager> mManager;
};

//inline implementations
//------------------------------------------------------------------------------

}
}