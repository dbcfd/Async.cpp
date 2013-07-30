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
        [](std::exception_ptr, size_t*, Series<size_t>::callback_t cb)-> void {
            cb(0);
        },
        [](std::exception_ptr ex, size_t* prev, Series<size_t>::callback_t cb)-> void {
            if(ex) std::rethrow_exception(ex);
            cb(*prev + 1);
        },
        [](std::exception_ptr ex, size_t* prev, Series<size_t>::callback_t cb)-> void {
            if(ex) std::rethrow_exception(ex);
            cb(*prev + 1);
        },
        [](std::exception_ptr ex, size_t* prev, Series<size_t>::callback_t cb)-> void {
            if(ex) std::rethrow_exception(ex);
            cb(*prev + 1);
        },
        [](std::exception_ptr ex, size_t* prev, Series<size_t>::callback_t cb)-> void {
            if(ex) std::rethrow_exception(ex);
            cb(*prev + 1);
        }
    };

    AsyncResult result;
    ASSERT_NO_THROW(result = Series<size_t>(manager, opsArray, 5).then(
        [](std::exception_ptr ex, size_t* prev)-> void {
            if(ex) std::rethrow_exception(ex);
            auto wasSuccessful = (4 == *prev);
            if(!wasSuccessful) throw(std::runtime_error("Series failed"));
        } ) );

    EXPECT_NO_THROW(result.check());

    manager->shutdown();
}

TEST(SERIES_TEST, INTERRUPT)
{
    auto manager(std::make_shared<tasks::AsioManager>(5));

    Series<size_t>::operation_t opsArray[] = {
        [](std::exception_ptr, size_t*, Series<size_t>::callback_t cb)-> void {
            cb(0);
        },
        [](std::exception_ptr ex, size_t* prev, Series<size_t>::callback_t cb)-> void {
            if(ex) std::rethrow_exception(ex);
            cb(*prev + 1);
        },
        [](std::exception_ptr ex, size_t* prev, Series<size_t>::callback_t cb)-> void {
            if(ex) std::rethrow_exception(ex);
            cb(*prev + 1);
        },
        [](std::exception_ptr ex, size_t* prev, Series<size_t>::callback_t cb)-> void {
            if(ex) std::rethrow_exception(ex);
            cb(*prev + 1);
        },
        [](std::exception_ptr ex, size_t* prev, Series<size_t>::callback_t cb)-> void {
            if(ex) std::rethrow_exception(ex);
            cb(*prev + 1);
        }
    };

    AsyncResult result;
    ASSERT_NO_THROW(result = Series<size_t>(manager, opsArray, 5).then(
        [](std::exception_ptr ex, size_t* prev)-> void {
            if(ex) std::rethrow_exception(ex);
            auto wasSuccessful = (4 == *prev);
            if(!wasSuccessful) throw(std::runtime_error("Series failed"));
        } ) );

    manager->shutdown();

    EXPECT_THROW(result.check(), std::runtime_error);
}

TEST(SERIES_TEST, TIMING)
{
    typedef std::chrono::high_resolution_clock::time_point data_t;
    auto manager(std::make_shared<tasks::AsioManager>(5));

    std::vector<data_t> times;
    times.reserve(6);
    auto func = [&times](std::exception_ptr ex, data_t*, Series<data_t>::callback_t callback)->void {
        if(ex) std::rethrow_exception(ex);
        auto time = std::chrono::high_resolution_clock::now();
        times.emplace_back(time); //no need to worry about locking since we're running serially
        callback(std::move(time));
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
    auto result = series.then([&times, &start](std::exception_ptr ex, data_t*)->void {
        if(ex) std::rethrow_exception(ex);
        times.emplace_back(std::chrono::high_resolution_clock::now()); //no need to worry about locking since we're running serially
    } );

    EXPECT_NO_THROW(result.check());

    auto last = times[0];

    for(size_t idx = 1; idx < 6; ++idx)
    {
        auto current = times[idx];
        ASSERT_LE(last, current);
        last = current;
    }

    manager->shutdown();
}