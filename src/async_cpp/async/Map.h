#pragma once
#include "async_cpp/async/Async.h"
#include "async_cpp/async/ParallelForEach.h"

#include <vector>

namespace async_cpp {
namespace async {

/**
 * Filter a set of data using a criteria
 */
template<class TDATA, class TRESULT>
class Map {
public:
    typedef typename std::function<OpResult<TRESULT>(TDATA&)> map_op_t;
    typedef typename ParallelForEach<TDATA>::complete_t callback_t;
    typedef std::function<void(OpResult<std::vector<TRESULT>>&&, typename Map<TDATA, TRESULT>::callback_t)> then_t;
    /**
     * Create a filter operation that will filter a set of data based on an operation.
     * @param manager Manager to use with filter operation
     * @param mapOp Operation to use for filtering
     * @param data Data to be mapped
     */
    Map(tasks::ManagerPtr manager, 
        typename map_op_t mapOp,
        std::vector<TDATA>&& data);

    /**
     * Run the operation across the set of data, invoking a task with the mapped results
     * @param afterMap Function to invoke when map operation is complete, receiving mapped data
     */
    std::future<AsyncResult> then(typename then_t afterMap);

    /**
     * Cancel any outstanding operations.
     */
    void cancel();

private:
    typename map_op_t mOp;
    tasks::ManagerPtr mManager;
    std::vector<TDATA> mData;
    std::shared_ptr<ParallelForEach<TDATA, TRESULT>> mParallel;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
Map<TDATA, TRESULT>::Map(tasks::ManagerPtr manager, 
                      typename map_op_t mapOp, 
                      std::vector<TDATA>&& data)
    : mManager(manager), mOp(mapOp), mData(std::move(data))
{

}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
std::future<AsyncResult> Map<TDATA, TRESULT>::then(typename then_t afterMap)
{
    auto op = [this](TDATA& value, typename ParallelForEach<TDATA, TRESULT>::callback_t cb) -> void {
        cb(OpResult<TRESULT>(mOp(value)));
    };
    mParallel = std::make_shared<ParallelForEach<TDATA, TRESULT>>(mManager, op, std::move(mData));
    return mParallel->then(
        [afterMap](OpResult<typename ParallelForEach<TDATA,TRESULT>::result_set_t>&& result, 
                    typename callback_t callback)->void 
        {
            if(result.wasError())
            {
                afterMap(OpResult<std::vector<TRESULT>>(result.error()), callback);
            }
            else
            {
                std::vector<TRESULT> mappedResults;
                auto results = result.move();
                mappedResults.reserve(results.size());
                for(auto& mappedResult : results)
                {
                    mappedResults.emplace_back(mappedResult.move());
                }
                results.clear();
                afterMap(OpResult<std::vector<TRESULT>>(std::move(mappedResults)), callback);
            }
        } );
}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
void Map<TDATA, TRESULT>::cancel()
{
    if(mParallel) mParallel->cancel();
}

}
}