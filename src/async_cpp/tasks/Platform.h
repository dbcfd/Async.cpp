#pragma once

#include <stdexcept>

#pragma warning(disable:4251)
#pragma warning(disable:4275)
//windows defines
#if defined(BUILD_SHARED_LIBS)
#if defined(Workers_EXPORTS)
#define WORKERS_API __declspec(dllexport)
#else
#define WORKERS_API __declspec(dllimport)
#endif
#else
#define WORKERS_API
#endif