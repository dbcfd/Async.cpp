#pragma once
#include "async_cpp/async/Async.h"

#include <future>

namespace async_cpp {
namespace async {

/**
 * Store the result of an asynchronous operation, either as an error or successful
 */
//------------------------------------------------------------------------------
class AsyncResult {
public:
    /**
     * Create a result that was valid, and waiting on completion
     */
    AsyncResult(std::future<bool>&& mFuture);
    /**
     * Create a finished, successful, async result
     */
    AsyncResult();
    
    virtual ~AsyncResult();

    /**
     * Check this asynchronous result. If failed, exception will be thrown.
     */
    void check();

    /**
     * Check if this result is ready.
     */
    bool isReady() const;

private:
    std::shared_ptr<std::future<bool>> mFuture;
};

//inline implementations
//------------------------------------------------------------------------------

}
}