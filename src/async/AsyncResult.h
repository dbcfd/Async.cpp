#pragma once
#include "async/Platform.h"

#include <memory>

namespace async_cpp {
namespace async {

/**
 * Store the result of an asynchronous operation, either as an error, or data.
 */
class ASYNC_API AsyncResult {
public:
    /**
     * Create a result that was an error
     * @param error String representing error
     */
    AsyncResult(const std::string& error);
    /**
     * Create a result that was valid, with data that will be retrieved later
     * @param data Shared point to data
     */
    AsyncResult(std::shared_ptr<void> data);
    /**
     * Create a result that was valid, but you don't wish to return a result
     */
    AsyncResult();
    
    virtual ~AsyncResult();

    /**
     * Check if this result is an error, throwing a std::runtime_error if it is. If not an error,
     * return the result.
     */
    std::shared_ptr<void> throwOrGet() const;
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
     * Return the result held by this object
     * @return Result held
     */
    inline std::shared_ptr<void> result() const;
    /**
     * Return the string that indicates errors
     * @return String holding error
     */
    inline const std::string& error() const;

    /**
     * Return the result as the given type, if not error. Throw if error.
     * @type Type of return
     * @return Shared pointer of type
     */
    template<class T>
    std::shared_ptr<T> throwOrAs() const;

private:
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

//------------------------------------------------------------------------------
template<class T>
std::shared_ptr<T> AsyncResult::throwOrAs() const
{
    return std::static_pointer_cast<T>(throwOrGet());
}

}
}