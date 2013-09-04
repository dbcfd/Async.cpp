#pragma once
#include "async_cpp/async/Async.h"
#include "async_cpp/async/ParallelFor.h"

#include <vector>

namespace async_cpp {
namespace async {

/**
 * Find unique results in a set of data using a comparison operator
 */
//------------------------------------------------------------------------------
template<class TDATA>
class Unique {
public:
    typedef typename std::function<bool(const TDATA&, const TDATA&)> equal_op_t;
    typedef typename ParallelFor<TDATA>::then_t then_t;
    /**
     * Create a filter operation that will filter a set of data based on an operation.
     * @param manager Manager to use with filter operation
     * @param equalOp Operation to use to determine if two data points are equal
     * @param data Data to be filtered
     */
    Unique(tasks::ManagerPtr manager, 
        typename equal_op_t equalOp,
        std::vector<TDATA>&& data);

    /**
     * Run the operation across the set of data, invoking a task with the unique results
     * @param onUnique Function to invoke when uniqueness operation is complete, receiving unique data
     */
    AsyncResult then(typename then_t onUnique);

    /**
     * Cancel outstanding tasks.
     */
    void cancel();

private:
    typename equal_op_t mOp;
    tasks::ManagerPtr mManager;
    std::vector<TDATA> mData;
    std::shared_ptr<ParallelFor<TDATA>> mParallel;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA>
Unique<TDATA>::Unique(tasks::ManagerPtr manager, 
                      typename equal_op_t equalOp, 
                      std::vector<TDATA>&& data)
    : mManager(manager), mOp(equalOp), mData(std::move(data))
{
    if(!mManager) { throw(std::invalid_argument("Unique: Manager cannot be null")); }
    if(mData.empty()) { throw(std::invalid_argument("Unique: Empty data set")); }
}

//------------------------------------------------------------------------------
template<class TDATA>
AsyncResult Unique<TDATA>::then(typename then_t onUnique)
{
    auto forSize = mData.size();
    auto saveData = std::make_shared<std::vector<TDATA>>(std::move(mData));
    auto equalOpCopy(mOp);
    auto op = [saveData, equalOpCopy](size_t index, typename ParallelFor<TDATA>::callback_t callback) -> void 
    {
        for(size_t i = 0; i < index; ++i)
        {
            if(equalOpCopy(saveData->at(i), saveData->at(index)))
            {
                //previous element in list matches this item, item is not the unique one
                callback(AsyncResult());
                return;
            }
        }
        callback(std::move(saveData->at(index)));
    };
    mParallel = std::make_shared<ParallelFor<TDATA>>(mManager, op, forSize);
    return mParallel->then(onUnique);
}

//------------------------------------------------------------------------------
template<class TDATA>
void Unique<TDATA>::cancel()
{
    if(mParallel) mParallel->cancel();
}

}
}