#pragma once
#include "workers/Platform.h"
#include "workers/Task.h"

#include <functional>

namespace async_cpp {
namespace workers {

/**
 * Basic task implementationwhich runs a function object.
 */
class WORKERS_API BasicTask : public Task {
public:
    /**
     * Create a task to run, which will run a function object
     * @param functionToRun Function object this task will run
     */
    BasicTask(std::function<void(void)> functionToRun);
    virtual ~BasicTask();

protected:
    virtual void performSpecific();

private:
    std::function<void(void)> mFunctionToRun;
};

//inline implementations
//------------------------------------------------------------------------------

}
}