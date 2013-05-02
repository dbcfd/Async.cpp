#pragma once
#include "workers/Platform.h"

#include <atomic>
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
     * Wait for this task to complete, returning whether or not the task was completed.
     * @return True if task completed successfully
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
    std::atomic<bool> mHasFulfilledPromise;
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