#include "async_cpp/async/AsyncResult.h"
#include "async_cpp/async/Series.h"

#include "async_cpp/tasks/AsioManager.h"

#include <chrono>

#pragma warning(disable:4251)
#include <gtest/gtest.h>

using namespace async_cpp;
using namespace async_cpp::async;

TEST(SERIES_TEST, BASIC)
{
    auto manager(std::make_shared<tasks::AsioManager>(5));

    Series<size_t>::operation_t opsArray[] = {
        [](OpResult<size_t>&&, Series<size_t>::callback_t cb)-> void {
            cb(OpResult<size_t>(0));
        },
        [](OpResult<size_t>&& result, Series<size_t>::callback_t cb)-> void {
            cb(OpResult<size_t>(result.throwOrMove() + 1));
        },
        [](OpResult<size_t>&& result, Series<size_t>::callback_t cb)-> void {
            cb(OpResult<size_t>(result.throwOrMove() + 1));
        },
        [](OpResult<size_t>&& result, Series<size_t>::callback_t cb)-> void {
            cb(OpResult<size_t>(result.throwOrMove() + 1));
        },
        [](OpResult<size_t>&& result, Series<size_t>::callback_t cb)-> void {
            cb(OpResult<size_t>(result.throwOrMove() + 1));
        }
    };

    auto future = Series<size_t>(manager, opsArray, 5).then(
        [](OpResult<size_t>&& result, Series<size_t>::complete_t cb)-> void {
            if(result.wasError())
            {
                cb(AsyncResult(result.error()));
            }
            else
            {
                auto wasSuccessful = (4 == result.move());
                if(wasSuccessful)
                {
                    cb(AsyncResult());
                }
                else
                {
                    cb(AsyncResult(std::string("Series failed")));
                }
            }
        } );

    AsyncResult asyncResult;
    ASSERT_NO_THROW(asyncResult = future.get());
    EXPECT_TRUE(asyncResult.wasSuccessful());

    manager->shutdown();
}

TEST(SERIES_TEST, INTERRUPT)
{
    auto manager(std::make_shared<tasks::AsioManager>(5));

    Series<size_t>::operation_t opsArray[] = {
        [](OpResult<size_t>&&, Series<size_t>::callback_t cb)-> void {
            cb(OpResult<size_t>(0));
        },
        [](OpResult<size_t>&& result, Series<size_t>::callback_t cb)-> void {
            cb(OpResult<size_t>(result.throwOrMove() + 1));
        },
        [](OpResult<size_t>&& result, Series<size_t>::callback_t cb)-> void {
            cb(OpResult<size_t>(result.throwOrMove() + 1));
        },
        [](OpResult<size_t>&& result, Series<size_t>::callback_t cb)-> void {
            cb(OpResult<size_t>(result.throwOrMove() + 1));
        },
        [](OpResult<size_t>&& result, Series<size_t>::callback_t cb)-> void {
            cb(OpResult<size_t>(result.throwOrMove() + 1));
        }
    };

    auto future = Series<size_t>(manager, opsArray, 5).then(
        [](OpResult<size_t>&& result, Series<size_t>::complete_t cb)-> void {
            if(result.wasError())
            {
                cb(AsyncResult(result.error()));
            }
            else
            {
                auto wasSuccessful = (4 == result.move());
                if(wasSuccessful)
                {
                    cb(AsyncResult());
                }
                else
                {
                    cb(AsyncResult(std::string("Series failed")));
                }
            }
        } );

    manager->shutdown();

    AsyncResult asyncResult;
    ASSERT_NO_THROW(asyncResult = future.get());
    EXPECT_FALSE(asyncResult.wasSuccessful());
}

TEST(SERIES_TEST, TIMING)
{
    typedef std::chrono::high_resolution_clock::time_point data_t;
    auto manager(std::make_shared<tasks::AsioManager>(5));

    std::vector<data_t> times;
    times.reserve(6);
    auto func = [&times](OpResult<data_t>&&, Series<data_t>::callback_t callback)->void {
        auto time = std::chrono::high_resolution_clock::now();
        times.emplace_back(time); //no need to worry about locking since we're running serially
        callback(OpResult<data_t>(std::move(time)));
    };

    Series<data_t>::operation_t opsArray[] = {
        func,
        func,
        func,
        func,
        func
    };

    Series<data_t> series(manager, opsArray, 5);
    auto start = std::chrono::high_resolution_clock::now();
    auto future = series.then([&times, &start](OpResult<data_t>&& result, Series<data_t>::complete_t callback)->void {
        if(result.wasError())
        {
            callback(AsyncResult(result.error()));
        }
        else
        {
            times.emplace_back(std::chrono::high_resolution_clock::now()); //no need to worry about locking since we're running serially
            callback(AsyncResult());
        }
    } );

    AsyncResult asyncResult;
    ASSERT_NO_THROW(asyncResult = future.get());
    EXPECT_TRUE(asyncResult.wasSuccessful());

    auto last = times[0];

    for(size_t idx = 1; idx < 6; ++idx)
    {
        auto current = times[idx];
        ASSERT_LE(last, current);
        last = current;
    }

    manager->shutdown();
}