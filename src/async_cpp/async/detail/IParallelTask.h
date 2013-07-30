#pragma once
#include "async_cpp/async/detail/IAsyncTask.h"

namespace async_cpp {
namespace async {
namespace detail {

/**
 * Common interface for parallel tasks
 */
template<class T>
class IParallelTask : public IAsyncTask<T> {
public:
    IParallelTask(std::weak_ptr<tasks::IManager> mgr);
    IParallelTask(IParallelTask&& other);
    virtual ~IParallelTask();

protected:
    IParallelTask(const IParallelTask& other);
};

//inline implementations
//------------------------------------------------------------------------------
template<class T>
IParallelTask<T>::IParallelTask(std::weak_ptr<tasks::IManager> mgr) 
    : IAsyncTask(mgr)
{
    
}

//------------------------------------------------------------------------------
template<class T>
IParallelTask<T>::IParallelTask(IParallelTask&& other) 
    : IAsyncTask(std::move(other))
{
    
}

//------------------------------------------------------------------------------
template<class T>
IParallelTask<T>::~IParallelTask()
{

}

}
}
}