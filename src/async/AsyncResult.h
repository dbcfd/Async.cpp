#pragma once
#include "async/Platform.h"

#include <memory>

namespace quicktcp {
namespace async {

class ASYNC_API AsyncResult {
public:
    /**
     * Create a result that was an error
     */
    AsyncResult(const std::string& error);
    /**
     * Create a result that was valid, with data that will be retrieved later
     */
    AsyncResult(std::shared_ptr<void> data);
    /**
     * Create a result that was valid, but you don't wish to return a result
     */
    AsyncResult();
    /**
     * Move constructor
     */
    AsyncResult(AsyncResult&& other);
    
    virtual ~AsyncResult();

    const bool forwardError(std::shared_ptr<AsyncResult> other);
    std::shared_ptr<void> getOrThrowIfError() const;
    void throwIfError() const;
    const bool wasError() const;

    inline std::shared_ptr<void> result() const;
    inline const std::string& error() const;

private:
    AsyncResult(const AsyncResult& other);

    std::string mError;
    std::shared_ptr<void> mResult;

};

//inline implementations
//------------------------------------------------------------------------------
std::shared_ptr<void> AsyncResult::result() const
{
    return mResult;
}

//------------------------------------------------------------------------------
const std::string& AsyncResult::error() const
{
    return mError;
}

}
}