#pragma once
#include "async_cpp/tasks/Tasks.h"

#include <chrono>
#include <memory>

namespace async_cpp {
namespace tasks {

/**
 * Interface for managers, allowing replacement/mocks.
 */
class ASYNC_CPP_TASKS_API IManager : public std::enable_shared_from_this<IManager> {
public:
    virtual ~IManager();

    /**
     * Run a task in this manager at next available time. If manager is shutdown, task will fail to perform.
     * @param task Task to run
     */
    virtual void run(std::shared_ptr<Task> task) = 0;

    /**
     * Run a task in this manager at a specified time. If manager is shutdown, task will fail to perform.
     * @param task Task to run
     */
    virtual void run(std::shared_ptr<Task> task, const std::chrono::high_resolution_clock::time_point& time) = 0;

    /**
     * Shutdown this manager. Any queued tasks will be marked as failing to complete.
     */
    virtual void shutdown() = 0;

    /**
     * Wait for all tasks running or queued to complete.
     */
    virtual void waitForTasksToComplete() = 0;

    /**
     * Flag indicating whether this manager is running. If not running, any tasks passed to it will be marked as failing to complete.
     * @return True if manager is running
     */
    virtual const bool isRunning() = 0;
};

//inline implementations
//------------------------------------------------------------------------------

}
}