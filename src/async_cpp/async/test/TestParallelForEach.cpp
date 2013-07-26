#include "async_cpp/async/AsyncResult.h"
#include "async_cpp/async/ParallelForEach.h"

#include "async_cpp/tasks/AsioManager.h"

#include <chrono>

#pragma warning(disable:4251)
#include <gtest/gtest.h>

using namespace async_cpp;
using namespace async_cpp::async;

TEST(PARALLEL_FOREACH_TEST, BASIC)
{
    typedef std::chrono::high_resolution_clock::time_point result_t;
    auto manager(std::make_shared<tasks::AsioManager>(5));
    std::vector<std::shared_ptr<std::chrono::high_resolution_clock::time_point>> times(6);

    auto func = [&times](size_t& index, ParallelForEach<size_t, result_t>::callback_t cb)->void {
        auto now = std::chrono::high_resolution_clock::now();
        times[index] = std::make_shared<std::chrono::high_resolution_clock::time_point>(now);
        cb(OpResult<result_t>(std::move(now)));
    };

    std::vector<size_t> data;
    data.emplace_back(0);
    data.emplace_back(1);
    data.emplace_back(2);
    data.emplace_back(3);
    data.emplace_back(4);

    ParallelForEach<size_t, result_t> parallel(manager, func, std::move(data));
    auto maxDur = std::chrono::high_resolution_clock::duration::min();
    auto start = std::chrono::high_resolution_clock::now();
    auto future = parallel.then([&times, &maxDur](OpResult<ParallelForEach<size_t, result_t>::result_set_t>&& result, ParallelForEach<size_t, result_t>::complete_t cb)->void {
        if(result.wasError())
        {
            cb(AsyncResult(result.error()));
        }
        else
        {
            auto results = result.move();
            for(auto& tp : results)
            {
                auto dur = std::chrono::high_resolution_clock::now() - tp.throwOrMove();
                maxDur = std::max(maxDur, dur);
            }
            cb(AsyncResult());
        }
    } );

    AsyncResult asyncResult;
    ASSERT_NO_THROW(asyncResult = future.get());
    auto totalDur = std::chrono::high_resolution_clock::now() - start;
    EXPECT_TRUE(asyncResult.wasSuccessful());

    ASSERT_GE(totalDur, maxDur);

    manager->shutdown();
}