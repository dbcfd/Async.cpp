#pragma once
#include "async_cpp/async/Async.h"
#include "async_cpp/async/ParallelForEach.h"

#include <vector>

namespace async_cpp {
namespace async {

/**
 * Filter a set of data using a criteria
 */
template<class TIN, class TOUT, class TRESULT = TOUT>
class Map {
public:
    /**
     * Create a filter operation that will filter a set of data based on an operation.
     * @param manager Manager to use with filter operation
     * @param mapOp Operation to use for filtering
     * @param data Data to be mapped
     */
    Map(std::shared_ptr<tasks::IManager> manager, 
        std::function<AsyncResult<TOUT>(std::shared_ptr<TIN>)> mapOp,
        const std::vector<std::shared_ptr<TIN>>& data);

    /**
     * Run the operation across the set of data, invoking a task with the mapped results
     * @param afterMap Function to invoke when map operation is complete, receiving mapped data
     */
    std::future<AsyncResult<TRESULT>> execute(std::function<std::future<AsyncResult<TRESULT>>(const AsyncResult<std::vector<std::shared_ptr<TOUT>>>&)> afterMap) const;

private:
    std::function<AsyncResult<TOUT>(std::shared_ptr<TIN>)> mOp;
    std::shared_ptr<tasks::IManager> mManager;
    std::vector<std::shared_ptr<TIN>> mData;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TIN, class TOUT, class TRESULT>
Map<TIN, TOUT, TRESULT>::Map(std::shared_ptr<tasks::IManager> manager, 
                      std::function<AsyncResult<TOUT>(std::shared_ptr<TIN>)> filterOp, 
                      const std::vector<std::shared_ptr<TIN>>& data)
    : mManager(manager), mOp(filterOp), mData(data)
{

}

//------------------------------------------------------------------------------
template<class TIN, class TOUT, class TRESULT>
std::future<AsyncResult<TRESULT>> Map<TIN, TOUT, TRESULT>::execute(std::function<std::future<AsyncResult<TRESULT>>(const AsyncResult<std::vector<std::shared_ptr<TOUT>>>&)> afterMap) const
{
    auto op = [this](std::shared_ptr<TIN> value) -> std::future<AsyncResult<TOUT>> {
        return AsyncResult<TOUT>(mOp(value)).asFulfilledFuture();
    };
    return ParallelForEach<TIN, TOUT, TRESULT>(mManager, op, mData).execute(
        [afterMap](const std::vector<AsyncResult<TOUT>>& results)->std::future<AsyncResult<TRESULT>> {
            try
            {
                std::vector<std::shared_ptr<TOUT>> mappedResults;
                mappedResults.reserve(results.size());
                for(auto result : results)
                {
                    auto value = result.throwOrGet();
                    if(value)
                    {
                        mappedResults.emplace_back(value);
                    }
                }
                return afterMap(AsyncResult<std::vector<std::shared_ptr<TOUT>>>(std::make_shared<std::vector<std::shared_ptr<TOUT>>>(std::move(mappedResults))));
            }
            catch(std::exception& ex)
            {
                return afterMap(AsyncResult<std::vector<std::shared_ptr<TOUT>>>(ex.what()));
            }
            catch(...)
            {
                return afterMap(AsyncResult<std::vector<std::shared_ptr<TOUT>>>("Map: Unknown error"));
            }
        } );
}

}
}