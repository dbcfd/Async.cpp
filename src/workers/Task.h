#pragma once
#include "workers/Platform.h"

#include <functional>
#include <future>

namespace quicktcp {
namespace workers {

class WORKERS_API Task {
public:
    Task();
    virtual ~Task();

    inline std::future<bool> getCompletionFuture();

    void perform(std::function<void(void)> priorToCompleteFunction);
    void failToPerform();

protected:
    virtual void performSpecific() = 0;

private:
    std::promise<bool> mTaskCompletePromise;
};

//inline implementations
//------------------------------------------------------------------------------
std::future<bool> Task::getCompletionFuture()
{
    return mTaskCompletePromise.get_future();
}

}
}