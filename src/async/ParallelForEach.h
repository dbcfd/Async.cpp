#pragma once
#include "async/Platform.h"
#include "async/AsyncTask.h"

namespace quicktcp {
namespace async {

/**
 * Perform an operation in parallel against all data in a vector, optionally calling a function to examine all results once parallel operations are complete.
 */
class ASYNC_API ParallelForEach {
public:
    /**
     * Create a parallel task set using a manager and a set of tasks.
     * @param manager Manager to run tasks against
     * @param tasks Vector of tasks that will be run
     */
    ParallelForEach(std::shared_ptr<workers::Manager> manager, 
        std::function<PtrAsyncResult(std::shared_ptr<void>)> op, 
        const std::vector<std::shared_ptr<void>>& data);

    /**
     * Run the operation across the set of data, invoking a task with the result of the data
     * @param onFinishTask Task to run when operation has been applied to all data
     * @return Future indicating when all operations (including onFinishTask) are complete
     */
    AsyncFuture execute(std::function<PtrAsyncResult(const std::vector<PtrAsyncResult>&)> onFinishTask);
    /**
     * Run the operation across the set of data.
     * @return Future indicating when all operations are complete
     */
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