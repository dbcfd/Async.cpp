#pragma once
#include "workers/Platform.h"

#include <functional>
#include <future>

namespace async_cpp {
namespace workers {

/**
 * Interface for tasks which will be run by a worker. If a task fails to perform successfully, completion future will be marked as false.
 */
class WORKERS_API Task {
public:
    Task();
    virtual ~Task();

    /**
     * Retrieve the future associated with this task which indicates when task is complete and whether or not it was successful.
     * @return Future associated with task
     */
    inline bool wasCompletedSuccessfully();

    /**
     * Perform the behavior of this task, invoking a function after the task complete promise is fulfilled.
     * @param afterCompleteFunction Function to invoke after task has fulfilled its promise
     */
    void perform(std::function<void(void)> afterCompleteFunction);

    /**
     * Mark this task as a failure by fulfilling its promise with false.
     */
    void failToPerform();

protected:
    virtual void performSpecific() = 0;

private:
    std::promise<bool> mTaskCompletePromise;
    std::future<bool> mTaskCompleteFuture;
};

//inline implementations
//------------------------------------------------------------------------------
bool Task::wasCompletedSuccessfully()
{
    return mTaskCompleteFuture.get();
}

}
}