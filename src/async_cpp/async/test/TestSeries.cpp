#include "async_cpp/async/AsyncResult.h"
#include "async_cpp/async/Series.h"

#include "async_cpp/tasks/Manager.h"
#ifdef HAS_BOOST
#include "async_cpp/tasks/AsioManager.h"
#endif

#include <chrono>

#pragma warning(disable:4251)
#include <gtest/gtest.h>

using namespace async_cpp;
using namespace async_cpp::async;

TEST(SERIES_TEST, BASIC)
{
    auto manager(std::make_shared<tasks::Manager>(5));

    std::function<std::future<AsyncResult<size_t>>(const AsyncResult<size_t>&)> opsArray[] = {
        [](const AsyncResult<size_t>& res)-> std::future<AsyncResult<size_t>> {
            return AsyncResult<size_t>(std::make_shared<size_t>(0)).asFulfilledFuture();
        },
        [](const AsyncResult<size_t>& res)->std::future<AsyncResult<size_t>> {
            auto previous = res.throwOrGet();
            return AsyncResult<size_t>(std::make_shared<size_t>(*previous + 1)).asFulfilledFuture();
        },
        [](const AsyncResult<size_t>& res)->std::future<AsyncResult<size_t>> {
            auto previous = res.throwOrGet();
            return AsyncResult<size_t>(std::make_shared<size_t>(*previous + 1)).asFulfilledFuture();
        },
        [](const AsyncResult<size_t>& res)->std::future<AsyncResult<size_t>> {
            auto previous = res.throwOrGet();
            return AsyncResult<size_t>(std::make_shared<size_t>(*previous + 1)).asFulfilledFuture();
        },
        [](const AsyncResult<size_t>& res)->std::future<AsyncResult<size_t>> {
            auto previous = res.throwOrGet();
            return AsyncResult<size_t>(std::make_shared<size_t>(*previous + 1)).asFulfilledFuture();
        }
    };

    auto future = Series<size_t, bool>(manager, opsArray, 5).execute(
        [](const AsyncResult<size_t>& result)->std::future<AsyncResult<bool>> {
            bool wasSuccessful = (4 == *result.throwOrGet());
            return AsyncResult<bool>(std::make_shared<bool>(wasSuccessful)).asFulfilledFuture();
        } );

    AsyncResult<bool> asyncResult;
    ASSERT_NO_THROW(asyncResult = future.get());
    std::shared_ptr<bool> result;
    ASSERT_NO_THROW(result = asyncResult.throwOrGet());
    ASSERT_TRUE(*result);

    manager->shutdown();
}

TEST(SERIES_TEST, TIMING)
{
    typedef std::chrono::high_resolution_clock::time_point data_t;
    auto manager(std::make_shared<tasks::Manager>(5));

    std::vector<std::shared_ptr<data_t>> times;
    times.reserve(6);
    auto func = [&times](const AsyncResult<data_t>& res)->std::future<AsyncResult<data_t>> {
        auto time = std::make_shared<data_t>(std::chrono::high_resolution_clock::now());
        times.emplace_back(time); //no need to worry about locking since we're running serially
        return AsyncResult<data_t>(time).asFulfilledFuture();
    };

    std::function<std::future<AsyncResult<data_t>>(const AsyncResult<data_t>&)> opsArray[] = {
        func,
        func,
        func,
        func,
        func
    };

    Series<data_t, bool> series(manager, opsArray, 5);
    auto start = std::chrono::high_resolution_clock::now();
    auto future = series.execute([&times, &start](const AsyncResult<data_t>& result)->std::future<AsyncResult<bool>> {
        auto time = std::make_shared<data_t>(std::chrono::high_resolution_clock::now());
        times.emplace_back(time); //no need to worry about locking since we're running serially
        return AsyncResult<bool>(std::make_shared<bool>(true)).asFulfilledFuture();
    } );

    AsyncResult<bool> asyncResult;
    ASSERT_NO_THROW(asyncResult = future.get());
    std::shared_ptr<bool> result;
    ASSERT_NO_THROW(result = asyncResult.throwOrGet());
    ASSERT_TRUE(result && *result);

    auto last = times[0];

    for(size_t idx = 1; idx < 6; ++idx)
    {
        auto current = times[idx];
        ASSERT_LE(*last, *current);
        last = current;
    }

    manager->shutdown();
}

#ifdef HAS_BOOST
TEST(SERIES_TEST, ASIO_TIMING)
{
    typedef std::chrono::high_resolution_clock::time_point data_t;
    auto manager(std::make_shared<tasks::AsioManager>(5));

    std::vector<std::shared_ptr<data_t>> times;
    times.reserve(6);
    auto func = [&times](const AsyncResult<data_t>& res)->std::future<AsyncResult<data_t>> {
        auto time = std::make_shared<data_t>(std::chrono::high_resolution_clock::now());
        times.emplace_back(time); //no need to worry about locking since we're running serially
        return AsyncResult<data_t>(time).asFulfilledFuture();
    };

    std::function<std::future<AsyncResult<data_t>>(const AsyncResult<data_t>&)> opsArray[] = {
        func,
        func,
        func,
        func,
        func
    };

    Series<data_t, bool> series(manager, opsArray, 5);
    auto start = std::chrono::high_resolution_clock::now();
    auto future = series.execute([&times, &start](const AsyncResult<data_t>& result)->std::future<AsyncResult<bool>> {
        auto time = std::make_shared<data_t>(std::chrono::high_resolution_clock::now());
        times.emplace_back(time); //no need to worry about locking since we're running serially
        return AsyncResult<bool>(std::make_shared<bool>(true)).asFulfilledFuture();
    } );

    AsyncResult<bool> asyncResult;
    ASSERT_NO_THROW(asyncResult = future.get());
    std::shared_ptr<bool> result;
    ASSERT_NO_THROW(result = asyncResult.throwOrGet());
    ASSERT_TRUE(result && *result);

    auto last = times[0];

    for(size_t idx = 1; idx < 6; ++idx)
    {
        auto current = times[idx];
        ASSERT_LE(*last, *current);
        last = current;
    }

    manager->shutdown();
}
#endif