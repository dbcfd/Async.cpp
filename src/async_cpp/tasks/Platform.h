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

#ifdef WINDOWS
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0502
#endif
#endif