#include "async_cpp/async/IParallelTask.h"

namespace async_cpp {
namespace async {

//------------------------------------------------------------------------------
IParallelTask::IParallelTask(std::shared_ptr<tasks::IManager> mgr) : mManager(mgr)
{

}

//------------------------------------------------------------------------------
IParallelTask::~IParallelTask()
{

}

}
}
