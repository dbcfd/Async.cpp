#include "async_cpp/async/AsyncResult.h"
#include "async_cpp/async/Parallel.h"
#include "async_cpp/async/ParallelFor.h"
#include "async_cpp/async/ParallelForEach.h"
#include "async_cpp/async/Series.h"

#include "async_cpp/tasks/AsioManager.h"

#include <chrono>

#pragma warning(disable:4251)
#include <gtest/gtest.h>

using namespace async_cpp;
using namespace async_cpp::async;

TEST(OVERLOAD_TEST, SERIES)
{
    typedef std::chrono::high_resolution_clock::time_point data_t;
    size_t nbTasks = 5;
    auto manager(std::make_shared<tasks::AsioManager>(3));
    std::vector<data_t> times;

    auto func = [&times](size_t, ParallelFor<data_t>::callback_t cb)->void {
        cb(std::chrono::high_resolution_clock::now());  
    };

    auto parallel5 = ParallelFor<data_t>(manager,
        [](size_t, ParallelFor<data_t>::callback_t cb)->void {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            cb(AsyncResult());
    }, 5).then( [](std::exception_ptr ex, std::vector<data_t>&&)->void { if(ex) std::rethrow_exception(ex); } );

    auto parallel25 = ParallelFor<data_t>(manager,
        [](size_t, ParallelFor<data_t>::callback_t cb)->void {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            cb(AsyncResult());
    }, 25).then( [](std::exception_ptr ex, std::vector<data_t>&&)->void { if(ex) std::rethrow_exception(ex); } );

    auto parallelTimes = ParallelFor<data_t>(manager, func, nbTasks).then(
        [&times](std::exception_ptr ex, std::vector<data_t>&& results)->void {
            if(ex) std::rethrow_exception(ex);
            for(auto& tp : results)
            {
                times.push_back(std::move(tp));
            }
    } );


    Series<int>::operation_t ops[] = {
        [&parallel5](std::exception_ptr, int*, Series<int>::callback_t cb)->void {
            cb(std::move(parallel5));
        },
        [&parallel25](std::exception_ptr ex, int*, Series<int>::callback_t cb)->void {
            if(ex) std::rethrow_exception(ex);
            cb(std::move(parallel25));
        }, 
        [](std::exception_ptr ex, int*, Series<int>::callback_t cb)->void {
            if(ex) std::rethrow_exception(ex);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            cb(AsyncResult());
        },
        [](std::exception_ptr ex, int*, Series<int>::callback_t cb)->void {
            if(ex) std::rethrow_exception(ex);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            cb(AsyncResult());
        },
        [&parallelTimes](std::exception_ptr ex, int*, Series<int>::callback_t cb)->void {
            if(ex) std::rethrow_exception(ex);
            cb(std::move(parallelTimes));
        }
    };

    auto result = Series<int>(manager, ops, 5).then([&times](std::exception_ptr ex, int*)->void { 
        if(ex) std::rethrow_exception(ex);
        times.emplace_back(std::chrono::high_resolution_clock::now());
    } );

    ASSERT_NO_THROW(result.check());

    for(size_t idx = 0; idx < nbTasks; ++idx)
    {
        ASSERT_LE(times[idx].time_since_epoch().count(), times[nbTasks].time_since_epoch().count());
    }

    manager->shutdown();
}

TEST(OVERLOAD_TEST, PARALLEL)
{
    typedef std::chrono::high_resolution_clock::time_point data_t;
    size_t nbTasks = 5;
    auto manager(std::make_shared<tasks::AsioManager>(3));
    std::vector<data_t> times;

    auto func = [&times](size_t, ParallelFor<data_t>::callback_t cb)->void {
        cb(std::chrono::high_resolution_clock::now());  
    };

    auto parallel5 = ParallelFor<data_t>(manager,
        [](size_t, ParallelFor<data_t>::callback_t cb)->void {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            cb(AsyncResult());
    }, 5).then( [](std::exception_ptr ex, std::vector<data_t>&&)->void { if(ex) std::rethrow_exception(ex); } );

    auto parallel25 = ParallelFor<data_t>(manager,
        [](size_t, ParallelFor<data_t>::callback_t cb)->void {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            cb(AsyncResult());
    }, 25).then( [](std::exception_ptr ex, std::vector<data_t>&&)->void { if(ex) std::rethrow_exception(ex); } );

    auto parallelTimes = ParallelFor<data_t>(manager, func, nbTasks).then(
        [&times](std::exception_ptr ex, std::vector<data_t>&& results)->void {
            if(ex) std::rethrow_exception(ex);
            for(auto& tp : results)
            {
                times.emplace_back(tp);
            }
    } );


    Parallel<int>::operation_t ops[] = {
        [&parallel5](Parallel<int>::callback_t cb)->void {
            cb(std::move(parallel5));
        },
        [&parallel25](Parallel<int>::callback_t cb)->void {
            cb(std::move(parallel25));
        }, 
        [](Parallel<int>::callback_t cb)->void {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            cb(AsyncResult());
        },
        [](Parallel<int>::callback_t cb)->void {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            cb(AsyncResult());
        },
        [&parallelTimes](Parallel<int>::callback_t cb)->void {
            cb(std::move(parallelTimes));
        }
    };

    auto result = Parallel<int>(manager, ops, 5).then( 
        [&times](std::exception_ptr ex, std::vector<int>&&)->void {
            if(ex) std::rethrow_exception(ex);

            times.emplace_back(std::chrono::high_resolution_clock::now());
    } );

    ASSERT_NO_THROW(result.check());

    for(size_t idx = 0; idx < nbTasks; ++idx)
    {
        ASSERT_LE(times[idx].time_since_epoch().count(), times[nbTasks].time_since_epoch().count());
    }

    manager->shutdown();
}