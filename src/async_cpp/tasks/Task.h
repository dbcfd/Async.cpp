#pragma once
#include "async_cpp/tasks/Tasks.h"

#include <atomic>
#include <functional>
#include <future>

namespace async_cpp {
namespace tasks {

/**
 * Interface for tasks which will be run by a worker. If a task fails to perform successfully, completion future will be marked as false.
 */
class ASYNC_CPP_TASKS_API Task : public std::enable_shared_from_this<Task> {
public:
    Task();
    virtual ~Task();

    /**
     * Check if this task is complete, returning whether or not the task was completed.
     * @return True if task is complete
     */
    inline bool isComplete();

    /**
     * Determine whether this task was successful. Will block until ready. Use isComplete to check if this call will block
     * @return True if task completed successfully
     */
    inline bool wasSuccessful();

    /**
     * Perform the behavior of this task
     */
    void perform();

    /**
     * Mark this task as a failure by fulfilling its promise with false.
     */
    void cancel();

protected:
    virtual void performSpecific() = 0;
    virtual void notifyCancel();
    virtual void notifyException(const std::exception& ex);

private:
    Task(const Task& other);

    void buildMembers();

    std::future<bool> mTaskCompleteFuture;
    std::atomic_bool mTaskInvoked;
    std::packaged_task<bool(bool)> mTask;
};

//inline implementations
//------------------------------------------------------------------------------
bool Task::isComplete()
{
#ifdef _MSC_VER //wait_for is broken in VC11 have to use MS specific _Is_ready
    return mTaskCompleteFuture._Is_ready();
#else
    return (std::future_status::ready == mTaskCompleteFuture.wait_for(std::chrono::milliseconds(0)));
#endif
}

//------------------------------------------------------------------------------
bool Task::wasSuccessful()
{
    return mTaskCompleteFuture.get();
}

}
}