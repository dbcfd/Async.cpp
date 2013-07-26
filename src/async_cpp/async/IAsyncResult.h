#pragma once
#include "async_cpp/async/Async.h"

namespace async_cpp {
namespace async {

/**
 * Basic interfaces for the asynchronous result types.
 */
class IAsyncResult {
public:
    /**
     * Create a result that was an error
     * @param error String representing error
     */
    IAsyncResult(std::string&& error);
    /**
     * Create a result that was not an error
     */
    IAsyncResult();
    /**
     * Move existing error
     */
    IAsyncResult(IAsyncResult&& other);
    /**
     * MOve existing result
     */
    IAsyncResult& operator=(IAsyncResult&& other);
    
    virtual ~IAsyncResult();

    /**
     * Throw a std::runtime_error if this result is an error
     */
    void throwIfError() const;
    /**
     * Indicate whether this result is an error.
     * @return True if this result is an error
     */
    const bool wasError() const;
    /**
     * Return the string that indicates errors
     * @return String holding error
     */
    std::string error() const;

private:
    IAsyncResult& operator=(const IAsyncResult& other);
    std::shared_ptr<std::string> mError;
};

//inline implementations
//------------------------------------------------------------------------------

}
}