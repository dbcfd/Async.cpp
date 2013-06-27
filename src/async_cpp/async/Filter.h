#pragma once
#include "async_cpp/async/Async.h"
#include "async_cpp/async/ParallelForEach.h"

#include <vector>

namespace async_cpp {
namespace async {

/**
 * Filter a set of data using a criteria
 */
template<class TDATA, class TRESULT = TDATA>
class Filter {
public:
    /**
     * Create a filter operation that will filter a set of data based on an operation.
     * @param manager Manager to use with filter operation
     * @param filterOp Operation to use for filtering
     * @param data Data to be filtered
     */
    Filter(std::shared_ptr<tasks::IManager> manager, 
        std::function<bool(std::shared_ptr<TDATA>)> filterOp,
        const std::vector<std::shared_ptr<TDATA>>& data);

    /**
     * Run the operation across the set of data, invoking a task with the filtered results
     * @param onFilter Function to invoke when filter operation is complete, receiving filtered data
     */
    std::future<AsyncResult<TRESULT>> execute(std::function<std::future<AsyncResult<TRESULT>>(const AsyncResult<std::vector<std::shared_ptr<TDATA>>>&)> onFilter) const;

private:
    std::function<bool(std::shared_ptr<TDATA>)> mOp;
    std::shared_ptr<tasks::IManager> mManager;
    std::vector<std::shared_ptr<TDATA>> mData;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
Filter<TDATA, TRESULT>::Filter(std::shared_ptr<tasks::IManager> manager, 
                      std::function<bool(std::shared_ptr<TDATA>)> filterOp, 
                      const std::vector<std::shared_ptr<TDATA>>& data)
    : mManager(manager), mOp(filterOp), mData(data)
{

}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
std::future<AsyncResult<TRESULT>> Filter<TDATA, TRESULT>::execute(std::function<std::future<AsyncResult<TRESULT>>(const AsyncResult<std::vector<std::shared_ptr<TDATA>>>&)> onFilter) const
{
    auto op = [this](std::shared_ptr<TDATA> value) -> std::future<AsyncResult<TDATA>> {
        if(mOp(value))
        {
            return AsyncResult<TDATA>(value).asFulfilledFuture();
        }
        else
        {
            return AsyncResult<TDATA>().asFulfilledFuture();
        }
    };
    return ParallelForEach<TDATA, TDATA, void>(mManager, op, mData).execute(
        [onFilter](const std::vector<AsyncResult<TDATA>>& results)->std::future<AsyncResult<TRESULT>> {
            try
            {
                std::vector<std::shared_ptr<TDATA>> filteredResults;
                filteredResults.reserve(results.size());
                for(auto result : results)
                {
                    auto value = result.throwOrGet();
                    if(value)
                    {
                        filteredResults.emplace_back(value);
                    }
                }
                filteredResults.shrink_to_fit();
                return onFilter(AsyncResult<std::vector<std::shared_ptr<TDATA>>>(std::make_shared<std::vector<std::shared_ptr<TDATA>>>(std::move(filteredResults))));
            }
            catch(std::exception& ex)
            {
                return onFilter(AsyncResult<std::vector<std::shared_ptr<TDATA>>>(ex.what()));
            }
            catch(...)
            {
                return onFilter(AsyncResult<std::vector<std::shared_ptr<TDATA>>>("Filter: Unknown error"));
            }
        } );
}

}
}