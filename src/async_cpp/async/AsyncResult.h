#pragma once
#include "async_cpp/async/IAsyncResult.h"

namespace async_cpp {
namespace async {

/**
 * Store the result of an asynchronous operation, either as an error or successful
 */
//------------------------------------------------------------------------------
class AsyncResult : public IAsyncResult {
public:
    /**
     * Create a result that was an error
     * @param error String representing error
     */
    AsyncResult(std::string&& error);
    /**
     * Create a result that was valid, but you don't wish to return a result
     */
    AsyncResult();
    /**
     * Create a result by moving another result
     */
    AsyncResult(AsyncResult&& other);
    /**
     * Create a result by moving another result
     */
    AsyncResult& operator=(AsyncResult&& other);
    
    virtual ~AsyncResult();

    /**
     * Check if this asynchronous result was successful, by throwing a runtime_error
     */
    void check() const;

    /**
     * Return whether this asynchronous result was successful.
     */
    bool wasSuccessful() const;

private:

};

//inline implementations
//------------------------------------------------------------------------------

}
}