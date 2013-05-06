#include "async_cpp/async/IParallelTask.h"

namespace async_cpp {
namespace async {

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
