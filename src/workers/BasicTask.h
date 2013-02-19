#pragma once
#include "workers/Platform.h"
#include "workers/Task.h"

#include <functional>

namespace quicktcp {
namespace workers {

class WORKERS_API BasicTask : public Task {
public:
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