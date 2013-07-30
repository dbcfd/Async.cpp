#pragma once
#include "async_cpp/async/ParallelForEach.h"

namespace async_cpp {
namespace async {

/**
 * Filter a set of data using a criteria
 */
//------------------------------------------------------------------------------
template<class TDATA>
class Filter {
public:
    typedef typename std::function<bool(const TDATA&)> filter_t;
    typedef typename ParallelForEach<TDATA>::then_t then_t;
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
    AsyncResult then(typename then_t onFilter);

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
AsyncResult Filter<TDATA>::then(typename then_t onFilter)
{
    auto filterOpCopy(mOp);
    auto op = [filterOpCopy](TDATA& value, typename detail::ParallelTask<TDATA>::callback_t callback) -> void {
        if(filterOpCopy(value))
        {
            callback(std::move(value));
        }
        else
        {
            callback(AsyncResult());
        }
    };
    mParallel = std::make_shared<ParallelForEach<TDATA>>(mManager, op, std::move(mData));
    return mParallel->then(onFilter);
}

//------------------------------------------------------------------------------
template<class TDATA>
void Filter<TDATA>::cancel()
{
    if(mParallel) mParallel->cancel();
}

}
}