#pragma once

#include <stdexcept>

#pragma warning(disable:4251)
#pragma warning(disable:4275)
//windows defines
#if defined(BUILD_SHARED_LIBS)
#if defined(Async_EXPORTS)
#define ASYNC_API __declspec(dllexport)
#else
#define ASYNC_API __declspec(dllimport)
#endif
#else
#define ASYNC_API
#endif