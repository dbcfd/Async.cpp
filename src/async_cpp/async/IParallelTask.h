#pragma once
#include "async_cpp/async/Async.h"

#include "async_cpp/tasks/Task.h"

namespace async_cpp {
namespace task {
class IManager;
}
namespace async {

/**
 * Common interface for parallel tasks
 */
class ASYNC_CPP_ASYNC_API IParallelTask : public tasks::Task {
public:
    IParallelTask(std::shared_ptr<tasks::IManager> mgr);
    virtual ~IParallelTask();

protected:
    IParallelTask(const IParallelTask& other);

    std::shared_ptr<tasks::IManager> mManager;
};

//inline implementations
//------------------------------------------------------------------------------

}
}