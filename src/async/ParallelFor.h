#pragma once
#include "async/Platform.h"
#include "async/AsyncTask.h"

namespace async_cpp {

namespace workers {
class IManager;
}

namespace async {

/**
 * Perform an operation in parallel for a number of times, optionally calling a function to examine all results once parallel 
 * operations are complete. Each task will be passed an index as data.
 */
class ASYNC_API ParallelFor {
public:
    /**
     * Create a parallel task set using a manager and a set of tasks.
     * @param manager Manager to run tasks against
     * @param op Operation to run in parallel for a number of times
     * @param nbTimes Number of times to run operation for
     */
    ParallelFor(std::shared_ptr<workers::IManager> manager, 
        std::function<AsyncResult(std::shared_ptr<void>)> op, 
        const size_t nbTimes);

    /**
     * Run the operation across the set of data, invoking a task with the result of the data
     * @param onFinishTask Task to run when operation has been applied to all data
     * @return Future indicating when all operations (including onFinishTask) are complete
     */
    AsyncFuture execute(std::function<AsyncResult(AsyncResult&)> onFinishTask);
    /**
     * Run the operation across the set of data.
     * @return Future indicating when all operations are complete
     */
    AsyncFuture execute();

private:
    std::function<AsyncResult(std::shared_ptr<void>)> mOp;
    std::shared_ptr<workers::IManager> mManager;
    size_t mNbTimes;
};

//inline implementations
//------------------------------------------------------------------------------

}
}