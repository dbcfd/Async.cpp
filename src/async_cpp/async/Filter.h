#pragma once
#include "async_cpp/async/ParallelForEach.h"

namespace async_cpp {
namespace async {

/**
 * Filter a set of data using a criteria
 */
template<class TDATA>
class Filter {
public:
    typedef typename std::function<bool(const TDATA&)> filter_t;
    typedef typename ParallelForEach<TDATA>::complete_t callback_t;
    typedef std::function<void(OpResult<std::vector<TDATA>>&&, typename Filter<TDATA>::callback_t)> then_t;
    /**
     * Create a filter operation that will filter a set of data based on an operation.
     * @param manager Manager to use with filter operation
     * @param filterOp Operation to use for filtering
     * @param data Data to be filtered
     */
    Filter(tasks::ManagerPtr manager, 
        typename filter_t filterOp,
        std::vector<TDATA>&& data);

    /**
     * Run the operation across the set of data, invoking a task with the filtered results
     * @param onFilter Function to invoke when filter operation is complete, receiving filtered data
     */
    std::future<AsyncResult> then(typename then_t onFilter);

    /**
     * Cancel any outstanding operations.
     */
    void cancel();

private:
    typename filter_t mOp;
    tasks::ManagerPtr mManager;
    std::vector<TDATA> mData;
    std::shared_ptr<ParallelForEach<TDATA>> mParallel;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA>
Filter<TDATA>::Filter(tasks::ManagerPtr manager, 
                      typename filter_t filterOp, 
                      std::vector<TDATA>&& data)
    : mManager(manager), mOp(filterOp), mData(std::move(data))
{

}

//------------------------------------------------------------------------------
template<class TDATA>
std::future<AsyncResult> Filter<TDATA>::then(typename then_t onFilter)
{
    auto op = [this](TDATA& value, typename detail::ParallelTask<TDATA>::callback_t callback) -> void {
        if(mOp(value))
        {
            callback(OpResult<TDATA>(std::move(value)));
        }
        else
        {
            callback(OpResult<TDATA>());
        }
    };
    mParallel = std::make_shared<ParallelForEach<TDATA>>(mManager, op, std::move(mData));
    return mParallel->then(
        [onFilter](OpResult<typename ParallelForEach<TDATA>::result_set_t>&& result, 
                    callback_t callback)->void
    {
        if(result.wasError())
        {
            onFilter(OpResult<std::vector<TDATA>>(result.error()), callback);
        }
        else
        {
            std::vector<TDATA> filteredResults;
            auto results = result.move();
            filteredResults.reserve(results.size());
            for(auto& filteredResult : results)
            {
                if(filteredResult.hasData())
                {
                    filteredResults.emplace_back(filteredResult.move());
                }
            }
            filteredResults.shrink_to_fit();
            onFilter(OpResult<std::vector<TDATA>>(std::move(filteredResults)), callback);
        }
    } );
}

//------------------------------------------------------------------------------
template<class TDATA>
void Filter<TDATA>::cancel()
{
    if(mParallel) mParallel->cancel();
}

}
}