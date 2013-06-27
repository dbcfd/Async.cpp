#pragma once
#include "async_cpp/async/Async.h"
#include "async_cpp/async/ParallelFor.h"

#include <vector>

namespace async_cpp {
namespace async {

/**
 * Find unique results in a set of data using a comparison operator
 */
template<class TDATA, class TRESULT = TDATA>
class Unique {
public:
    /**
     * Create a filter operation that will filter a set of data based on an operation.
     * @param manager Manager to use with filter operation
     * @param equalOp Operation to use to determine if two data points are equal
     * @param data Data to be filtered
     */
    Unique(std::shared_ptr<tasks::IManager> manager, 
        std::function<bool(std::shared_ptr<TDATA>, std::shared_ptr<TDATA>)> equalOp,
        const std::vector<std::shared_ptr<TDATA>>& data);

    /**
     * Run the operation across the set of data, invoking a task with the unique results
     * @param onUnique Function to invoke when uniqueness operation is complete, receiving unique data
     */
    std::future<AsyncResult<TRESULT>> execute(std::function<std::future<AsyncResult<TRESULT>>(const AsyncResult<std::vector<std::shared_ptr<TDATA>>>&)> onUnique) const;

private:
    std::function<bool(std::shared_ptr<TDATA>, std::shared_ptr<TDATA>)> mOp;
    std::shared_ptr<tasks::IManager> mManager;
    std::vector<std::shared_ptr<TDATA>> mData;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
Unique<TDATA, TRESULT>::Unique(std::shared_ptr<tasks::IManager> manager, 
                      std::function<bool(std::shared_ptr<TDATA>, std::shared_ptr<TDATA>)> uniqueOp, 
                      const std::vector<std::shared_ptr<TDATA>>& data)
    : mManager(manager), mOp(uniqueOp), mData(data)
{

}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
std::future<AsyncResult<TRESULT>> Unique<TDATA, TRESULT>::execute(std::function<std::future<AsyncResult<TRESULT>>(const AsyncResult<std::vector<std::shared_ptr<TDATA>>>&)> onUnique) const
{
    auto saveData = std::make_shared<std::vector<std::shared_ptr<TDATA>>>(mData);
    auto op = [saveData, this](size_t index) -> std::future<AsyncResult<TDATA>> {
        for(size_t i = 0; i < index; ++i)
        {
            if(mOp(saveData->at(i), saveData->at(index)))
            {
                //previous element in list matches this item, item is not the unique one
                return AsyncResult<TDATA>().asFulfilledFuture();
            }
        }
        return AsyncResult<TDATA>(saveData->at(index)).asFulfilledFuture();
    };
    return ParallelFor<TDATA, TRESULT>(mManager, op, mData.size()).execute(
        [onUnique](const std::vector<AsyncResult<TDATA>>& results)->std::future<AsyncResult<TRESULT>> {
            try
            {
                std::vector<std::shared_ptr<TDATA>> uniqueResults;
                uniqueResults.reserve(results.size());
                for(auto result : results)
                {
                    auto value = result.throwOrGet();
                    if(value)
                    {
                        uniqueResults.emplace_back(value);
                    }
                }
                uniqueResults.shrink_to_fit();
                return onUnique(AsyncResult<std::vector<std::shared_ptr<TDATA>>>(std::make_shared<std::vector<std::shared_ptr<TDATA>>>(std::move(uniqueResults))));
            }
            catch(std::exception& ex)
            {
                return onUnique(AsyncResult<std::vector<std::shared_ptr<TDATA>>>(ex.what()));
            }
            catch(...)
            {
                return onUnique(AsyncResult<std::vector<std::shared_ptr<TDATA>>>("Unique: Unknown error"));
            }
        } );
}

}
}