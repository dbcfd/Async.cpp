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
     * Run the operation across the set of data, invoking a task with the filtered results
     * @param onFilter Function to invoke when filter operation is complete, receiving filtered data
     */
    std::future<AsyncResult<TRESULT>> execute(std::function<std::future<AsyncResult<TRESULT>>(const AsyncResult<std::vector<std::shared_ptr<TOUT>>>&)> onMap) const;

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
std::future<AsyncResult<TRESULT>> Map<TIN, TOUT, TRESULT>::execute(std::function<std::future<AsyncResult<TRESULT>>(const AsyncResult<std::vector<std::shared_ptr<TOUT>>>&)> onFilter) const
{
    auto op = [this](std::shared_ptr<TIN> value) -> std::future<AsyncResult<TOUT>> {
        return AsyncResult<TOUT>(mOp(value)).asFulfilledFuture();
    };
    return ParallelForEach<TIN, TOUT, void>(mManager, op, mData).execute(
        [onFilter](const std::vector<AsyncResult<TDATA>>& results)->std::future<AsyncResult<TRESULT>> {
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
                return onFilter(AsyncResult<std::vector<std::shared_ptr<TOUT>>>(std::make_shared<std::vector<std::shared_ptr<TOUT>>>(std::move(filteredResults))));
            }
            catch(std::exception& ex)
            {
                return onFilter(AsyncResult<std::vector<std::shared_ptr<TDATA>>>(ex.what()));
            }
            catch(...)
            {
                return onFilter(AsyncResult<std::vector<std::shared_ptr<TDATA>>>("Map: Unknown error"));
            }
        } );
}

}
}