#include "async_cpp/async/AsyncResult.h"
#include "async_cpp/async/ParallelFor.h"

#include "async_cpp/tasks/AsioManager.h"

#include <chrono>

#pragma warning(disable:4251)
#include <gtest/gtest.h>

using namespace async_cpp;
using namespace async_cpp::async;

TEST(PARALLEL_FOR_TEST, BASIC)
{
    typedef std::chrono::high_resolution_clock::time_point data_t;
    typedef std::shared_ptr<data_t> data_ptr_t;
    auto manager(std::make_shared<tasks::AsioManager>(5));
    std::vector<data_ptr_t> times(6);

    auto func = [&times](size_t index, ParallelFor<data_t>::callback_t cb)->void {
        auto now = std::chrono::high_resolution_clock::now();
        times[index] = std::make_shared<data_t>(now);
        cb(std::move(now));
    };

    ParallelFor<data_t> parallel(manager, func, 5);
    auto maxDur = std::chrono::high_resolution_clock::duration::min();
    auto start = std::chrono::high_resolution_clock::now();
    auto result = parallel.then([&times, maxDur, this](std::exception_ptr ex, std::vector<data_t>&& results)->void {
        if(ex) std::rethrow_exception(ex);

        for(size_t i = 0; i < results.size(); ++i)
        {
            auto& tp = results[i];
            auto prev = times[i];
            if(!prev)
            {
                throw(std::runtime_error("No previous time"));
            }
            if(*prev != tp)
            {
                throw(std::runtime_error("Time mismatch"));
            }
        }
    } );

    ASSERT_NO_THROW(result.check());

    manager->shutdown();
}