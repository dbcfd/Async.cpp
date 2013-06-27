#include "async_cpp/async/detail/IParallelTask.h"

namespace async_cpp {
namespace async {
namespace detail {

//------------------------------------------------------------------------------
IParallelTask::IParallelTask(std::weak_ptr<tasks::IManager> mgr) : Task(), mManager(mgr)
{
    
}

//------------------------------------------------------------------------------
IParallelTask::~IParallelTask()
{

}

}
}
}
