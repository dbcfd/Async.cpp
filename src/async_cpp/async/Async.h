#pragma once

#include "async_cpp/async/Platform.h"
#include "async_cpp/tasks/Tasks.h"

#ifndef _MSC_VER
#include <future>
#endif

namespace async_cpp {
namespace async {

template<class TDATA> class AsyncResult;

//if not using vc11, can use these aliases to simplify your code
#ifndef _MSC_VER
template<class TDATA> 
using AsyncPromise = std::promise<AsyncResult<TDATA>>;
template<class TDATA>
using std::future<AsyncResult<TDATA>> = std::future<AsyncResult<TDATA>>;
#endif

}
}