#pragma once
#include "async_cpp/async/Async.h"

#include "async_cpp/tasks/Task.h"

namespace async_cpp {
namespace async {
namespace detail {

/**
 * Common interface for parallel tasks
 */
class IParallelTask : public tasks::Task {
public:
    IParallelTask(std::weak_ptr<tasks::IManager> mgr);
    virtual ~IParallelTask();

protected:
    IParallelTask(const IParallelTask& other);

    std::weak_ptr<tasks::IManager> mManager;
};

//inline implementations
//------------------------------------------------------------------------------

}
}
}