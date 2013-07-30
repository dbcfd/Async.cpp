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

    auto func = [&times](const size_t& index, ParallelForEach<size_t, result_t>::callback_t cb)->void {
        auto now = std::chrono::high_resolution_clock::now();
        times[index] = std::make_shared<std::chrono::high_resolution_clock::time_point>(now);
        cb(std::move(now));
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
    auto result = parallel.then([&times, &maxDur](std::exception_ptr ex, std::vector<result_t>&& results)->void {
        if(ex) std::rethrow_exception(ex);

        for(auto& tp : results)
        {
            auto dur = std::chrono::high_resolution_clock::now() - tp;
            maxDur = std::max(maxDur, dur);
        }
    } );

    ASSERT_NO_THROW(result.check());
    auto totalDur = std::chrono::high_resolution_clock::now() - start;

    ASSERT_GE(totalDur, maxDur);

    manager->shutdown();
}