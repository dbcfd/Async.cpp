#pragma once
#include "async_cpp/async/AsyncResult.h"

#include <boost/variant/static_visitor.hpp>
#include <vector>

namespace async_cpp
{
namespace async
{
namespace detail
{

/**
 * Task which collects a set of futures from parallel tasks.
 */
//------------------------------------------------------------------------------
template<class T>
class ValueVisitor : public boost::static_visitor<T*> {
public:
    T* operator()(T& result) const;
    T* operator()(AsyncResult& as) const;
    T* operator()(std::exception_ptr p) const;
};


//inline implementations
//------------------------------------------------------------------------------
template<class T>
T* ValueVisitor<T>::operator()(T& result) const
{
    return &result;
}

//------------------------------------------------------------------------------
template<class T>
T* ValueVisitor<T>::operator()(AsyncResult& as) const
{
    as.check();
    return nullptr;
}

//------------------------------------------------------------------------------
template<class T>
T* ValueVisitor<T>::operator()(std::exception_ptr p) const
{
    std::rethrow_exception(p);
    return nullptr;
}

}
}
}