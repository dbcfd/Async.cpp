#pragma once

#include "async_cpp/tasks/Platform.h"

#include <memory>

namespace async_cpp {
namespace tasks {

class IManager;
typedef std::shared_ptr<IManager> ManagerPtr;
class Task;

}
}