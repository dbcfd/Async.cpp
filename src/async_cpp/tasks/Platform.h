#pragma once

#pragma warning(disable:4251)
#pragma warning(disable:4275)
//windows defines
#if defined(BUILD_SHARED_LIBS)
#if defined(Tasks_EXPORTS)
#define ASYNC_CPP_TASKS_API __declspec(dllexport)
#else
#define ASYNC_CPP_TASKS_API __declspec(dllimport)
#endif
#else
#define ASYNC_CPP_TASKS_API
#endif