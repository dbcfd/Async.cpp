#pragma once
#include "async_cpp/async/AsyncResult.h"

#include <boost/variant/static_visitor.hpp>

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
class ReadyVisitor : public boost::static_visitor<bool> {
public:
    bool operator()(const T&) const;
    bool operator()(const AsyncResult& value) const;
    bool operator()(std::exception_ptr) const;
};

//inline implementations
//------------------------------------------------------------------------------
template<class T>
bool ReadyVisitor<T>::operator()(const T&) const
{
    return true;
}

//------------------------------------------------------------------------------
template<class T>
bool ReadyVisitor<T>::operator()(const AsyncResult& value) const
{
    return value.isReady();
}

//------------------------------------------------------------------------------
template<class T>
bool ReadyVisitor<T>::operator()(std::exception_ptr) const
{
    return true;
}

}
}
}