#include "async/Series.h"
#include "async/AsyncResult.h"
#include "async/AsyncTask.h"

#include "workers/IManager.h"

#include <assert.h>

namespace async_cpp {
namespace async {

//------------------------------------------------------------------------------
Series::Series(std::shared_ptr<workers::IManager> manager, const std::vector<std::function<AsyncResult(AsyncResult&)>>& ops)
    : mManager(manager), mOperations(ops)
{
    assert(nullptr != manager);
}

//------------------------------------------------------------------------------
Series::Series(std::shared_ptr<workers::IManager> manager, std::function<AsyncResult(AsyncResult&)>* ops, const size_t nbOps)
    : mManager(manager)
{
    assert(nullptr != manager);
    assert(nullptr != ops);

    mOperations.assign(ops, ops+nbOps);
}

//------------------------------------------------------------------------------
AsyncFuture Series::execute(std::function<AsyncResult(AsyncResult&)> onFinishOp)
{
    std::function<AsyncResult(AsyncResult&)> finishOp = [onFinishOp](AsyncResult& input)->AsyncResult {
            AsyncResult result;
            try {
                result = std::move(onFinishOp(input));
            }
            catch(...)
            {
                result = std::move(AsyncResult("Unknown error"));
            }
            return result;
        };
    std::shared_ptr<AsyncTerminalTask> finishTask(new AsyncTerminalTask(finishOp));

    if(mOperations.empty())
    {
        mManager->run(finishTask);
    }
    else
    {
        auto op = mOperations.rbegin();
        std::shared_ptr<AsyncForwardTask> lastTask;

        std::function<void(AsyncResult&)> func = std::bind(
            [finishTask](std::shared_ptr<workers::IManager> mgr, std::function<AsyncResult(AsyncResult&)>& op, AsyncResult& input)->void {
                auto result = op(input);
                finishTask->forward(result);
                mgr->run(finishTask);
            }, mManager, std::move(*op), std::placeholders::_1);

        std::shared_ptr<AsyncForwardTask> curTask( new AsyncForwardTask(func) );

        lastTask = curTask;

        for(op = mOperations.rbegin() + 1; op != mOperations.rend(); ++op)
        {
            func = std::bind(
                [lastTask](std::shared_ptr<workers::IManager> mgr, std::function<AsyncResult(AsyncResult&)>& op, AsyncResult& input)->void {
                    auto result = op(input);
                    lastTask->forward(result);
                    mgr->run(lastTask);
                }, mManager, std::move(*op), std::placeholders::_1 );
            std::shared_ptr<AsyncForwardTask> curTask( new AsyncForwardTask( func ) );
            lastTask = curTask;
        }
        mManager->run(lastTask);
    }

    return finishTask->future();  
}

//------------------------------------------------------------------------------
AsyncFuture Series::execute()
{
    std::function<AsyncResult(AsyncResult&)> func = [](AsyncResult& in)->AsyncResult { return std::move(in); };
    return execute(func);
}

}
}
