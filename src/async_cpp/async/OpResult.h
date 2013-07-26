#pragma once
#include "async_cpp/async/IAsyncResult.h"

#include <memory>

namespace async_cpp {
namespace async {

/**
 * Store the result of an operation within an asynchronous call, either as an error, or data.
 */
template<class TDATA>
class OpResult : public IAsyncResult {
public:
    /**
     * Create a result that was an error
     * @param error String representing error
     */
    OpResult(std::string&& error);
    /**
     * Create a result that was valid by moving data into a shared pointer that will be retrieved later
     * @param data Movable data
     */
    OpResult(TDATA&& data);
    /**
     * Create a result that was valid, but you don't wish to return a result
     */
    OpResult();
    /**
     * Move a pre-existing op result
     */
    OpResult(OpResult&& other);
    /**
     * Move a pre-existing op result
     */
    OpResult& operator=(OpResult&& other);
    
    virtual ~OpResult();

    /**
     * Check if this result is an error, throwing a std::runtime_error if it is. If not an error,
     * return the result.
     */
    TDATA throwOrMove();
    /**
     * Return the result held by this object
     * @return Result held
     */
    TDATA move();
    /**
     * If this object has movable data.
     */
    bool hasData() const;

private:
    OpResult& operator=(const OpResult& other);
    std::unique_ptr<TDATA> mData;

};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA>
OpResult<TDATA>::OpResult(std::string&& error) : IAsyncResult(std::move(error))
{
    
}

//------------------------------------------------------------------------------
template<class TDATA>
OpResult<TDATA>::OpResult() : IAsyncResult()
{

}
    
//------------------------------------------------------------------------------
template<class TDATA>
OpResult<TDATA>::OpResult(TDATA&& data) : IAsyncResult(), mData(new TDATA(std::move(data)))
{

}

//------------------------------------------------------------------------------
template<class TDATA>
OpResult<TDATA>::OpResult(OpResult&& other) : IAsyncResult(std::move(other)), mData(std::move(other.mData))
{

}

//------------------------------------------------------------------------------
template<class TDATA>
OpResult<TDATA>& OpResult<TDATA>::operator=(OpResult&& other)
{
    IAsyncResult::operator=(std::move(other));
    mData = std::move(other.mData);
    return *this;
}

//------------------------------------------------------------------------------
template<class TDATA>
OpResult<TDATA>::~OpResult()
{

}

//------------------------------------------------------------------------------
template<class TDATA>
TDATA OpResult<TDATA>::throwOrMove()
{
    throwIfError();
    return move();
}

//------------------------------------------------------------------------------
template<class TDATA>
TDATA OpResult<TDATA>::move()
{
    if(!mData)
    {
        throw(std::runtime_error("OpResult: Attempting to move value from a result with value"));
    }
    auto result = std::move(*mData);
    mData.reset();
    return result;
}

//------------------------------------------------------------------------------
template<class TDATA>
bool OpResult<TDATA>::hasData() const
{
    return (nullptr != mData);
}

}
}