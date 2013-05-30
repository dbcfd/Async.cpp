#pragma once
#include "async_cpp/tasks/Tasks.h"
#include "async_cpp/tasks/Task.h"

#include <functional>

namespace async_cpp {
namespace tasks {

/**
 * Implementation of a task which takes a function to run, and runs that during performSpecific.
 */
class ASYNC_CPP_TASKS_API BasicTask : public Task {
public:
    /**
     * Create a task to run, which will run a function object
     * @param functionToRun Function object this task will run
     */
    BasicTask(std::function<void(void)> functionToRun);
    virtual ~BasicTask();

protected:
    virtual void performSpecific() final;
    virtual void notifyFailureToPerform() final;
    virtual void onException(const std::exception& ex) final;

private:
    std::function<void(void)> mFunctionToRun;
};

//inline implementations
//------------------------------------------------------------------------------

}
}