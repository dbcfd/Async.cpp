#pragma once
#include "async_cpp/async/Async.h"

#include "async_cpp/tasks/Task.h"

#include <boost/variant.hpp>

namespace async_cpp {
namespace async {
namespace detail {

/**
 * Common interface for all async tasks
 */
template<class T>
class IAsyncTask : public tasks::Task {
public:
    typedef typename boost::variant<
        T,
        AsyncResult,
        std::exception_ptr
    > VariantType;
    IAsyncTask(std::weak_ptr<tasks::IManager> mgr);
    IAsyncTask(IAsyncTask&& other);
    virtual ~IAsyncTask();

protected:
    IAsyncTask(const IAsyncTask& other);

    virtual void attemptOperation(std::function<void(void)> op) = 0;

    std::weak_ptr<tasks::IManager> mManager;
};

//inline implementations
//------------------------------------------------------------------------------
template<class T>
IAsyncTask<T>::IAsyncTask(std::weak_ptr<tasks::IManager> mgr) : Task(), mManager(mgr)
{
    
}

//------------------------------------------------------------------------------
template<class T>
IAsyncTask<T>::IAsyncTask(IAsyncTask&& other) : mManager(other.mManager)
{
    
}

//------------------------------------------------------------------------------
template<class T>
IAsyncTask<T>::~IAsyncTask()
{

}

}
}
}