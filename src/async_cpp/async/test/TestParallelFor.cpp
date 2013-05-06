#include "async_cpp/async/AsyncResult.h"
#include "async_cpp/async/ParallelFor.h"

#include "async_cpp/tasks/Manager.h"

#include <chrono>

#pragma warning(disable:4251)
#include <gtest/gtest.h>

using namespace async_cpp;
using namespace async_cpp::async;

TEST(PARALLEL_FOR_TEST, BASIC)
{
    typedef std::chrono::high_resolution_clock::duration result_t;
    auto manager(std::make_shared<tasks::Manager>(5));
    std::vector<std::shared_ptr<std::chrono::high_resolution_clock::time_point>> times(6);

    auto func = [&times](size_t index)->std::future<AsyncResult<size_t>> {
        times[index] = std::make_shared<std::chrono::high_resolution_clock::time_point>(std::chrono::high_resolution_clock::now());
        return AsyncResult<size_t>(std::make_shared<size_t>(index)).asFulfilledFuture();
    };

    ParallelFor<size_t, result_t> parallel(manager, func, 5);
    auto start = std::chrono::high_resolution_clock::now();
    auto future = parallel.execute([&times](const std::vector<AsyncResult<size_t>>& results)->std::future<AsyncResult<result_t>> {
        auto maxDur = std::chrono::high_resolution_clock::duration::min();
        for(auto& result : results)
        {
            auto dur = std::chrono::high_resolution_clock::now() - *(times[*(result.throwOrGet())]);
            maxDur = std::max(maxDur, dur);
        }
        return AsyncResult<result_t>(std::make_shared<result_t>(maxDur)).asFulfilledFuture();
    } );

    AsyncResult<result_t> asyncResult;
    ASSERT_NO_THROW(asyncResult = future.get());
    auto totalDur = std::chrono::high_resolution_clock::now() - start;

    std::shared_ptr<result_t> result;
    ASSERT_NO_THROW(result = asyncResult.throwOrGet());
    ASSERT_TRUE(result);
    ASSERT_GE(totalDur, *result);

    manager->shutdown();
}