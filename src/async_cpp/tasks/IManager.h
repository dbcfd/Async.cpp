#pragma once
#include "workers/Platform.h"

#include <memory>

namespace async_cpp {
namespace workers {

class Task;

/**
 * Interface for managers, allowing replacement/mocks.
 */
class WORKERS_API IManager {
public:
    IManager();
    virtual ~IManager();

    /**
     * Run a task on the first available worker, queueing if none are available.
     * @param task Task to run
     */
    virtual void run(std::shared_ptr<Task> task) = 0;

    /**
     * Shutdown this manager, including all workers. Any queued tasks will be marked as failing to complete.
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

    /**
     * Size indicating best possible chunk size for a large set of threads.
     * @return Best possible size for thread chunk
     */
    virtual const size_t chunkSize() =  0;
};

//inline implementations
//------------------------------------------------------------------------------

}
}