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
        cb(OpResult<data_t>(std::chrono::high_resolution_clock::now()));  
    };

    auto parallel5 = ParallelFor<data_t>(manager,
        [](size_t, ParallelFor<data_t>::callback_t cb)->void {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            cb(OpResult<data_t>());
    }, 5).then( [](OpResult<ParallelFor<data_t>::result_set_t>&&, ParallelFor<data_t>::complete_t cb)->void { cb(AsyncResult()); } );

    auto parallel25 = ParallelFor<data_t>(manager,
        [](size_t, ParallelFor<data_t>::callback_t cb)->void {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            cb(OpResult<data_t>());
    }, 25).then( [](OpResult<ParallelFor<data_t>::result_set_t>&&, ParallelFor<data_t>::complete_t cb)->void { cb(AsyncResult()); } );

    auto parallelTimes = ParallelFor<data_t>(manager, func, nbTasks).then(
        [&times](OpResult<ParallelFor<data_t>::result_set_t>&& result, ParallelFor<data_t>::complete_t cb)->void {
            auto results = result.throwOrMove();
            for(auto& tp : results)
            {
                times.emplace_back(tp.throwOrMove());
            }
            cb(AsyncResult());
    } );


    Series<int>::operation_t ops[] = {
        [&parallel5](OpResult<int>&&, Series<int>::callback_t cb)->void {
            parallel5.get().check();
            cb(OpResult<int>());
        },
        [&parallel25](OpResult<int>&&, Series<int>::callback_t cb)->void {
            parallel25.get().check();
            cb(OpResult<int>());
        }, 
        [](OpResult<int>&&, Series<int>::callback_t cb)->void {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            cb(OpResult<int>());
        },
        [](OpResult<int>&&, Series<int>::callback_t cb)->void {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            cb(OpResult<int>());
        },
        [&parallelTimes](OpResult<int>&&, Series<int>::callback_t cb)->void {
            parallelTimes.get().check();
            cb(OpResult<int>());
        }
    };

    auto result = Series<int>(manager, ops, 5).then([&times](OpResult<int>&&, Series<int>::complete_t cb)->void { 
        times.emplace_back(std::chrono::high_resolution_clock::now());
        cb(AsyncResult()); 
    } );

    ASSERT_NO_THROW(result.get());

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
        cb(OpResult<data_t>(std::chrono::high_resolution_clock::now()));  
    };

    auto parallel5 = ParallelFor<data_t>(manager,
        [](size_t, ParallelFor<data_t>::callback_t cb)->void {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            cb(OpResult<data_t>());
    }, 5).then( [](OpResult<ParallelFor<data_t>::result_set_t>&&, ParallelFor<data_t>::complete_t cb)->void { cb(AsyncResult()); } );

    auto parallel25 = ParallelFor<data_t>(manager,
        [](size_t, ParallelFor<data_t>::callback_t cb)->void {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            cb(OpResult<data_t>());
    }, 25).then( [](OpResult<ParallelFor<data_t>::result_set_t>&&, ParallelFor<data_t>::complete_t cb)->void { cb(AsyncResult()); } );

    auto parallelTimes = ParallelFor<data_t>(manager, func, nbTasks).then(
        [&times](OpResult<ParallelFor<data_t>::result_set_t>&& result, ParallelFor<data_t>::complete_t cb)->void {
            auto results = result.throwOrMove();
            for(auto& tp : results)
            {
                times.emplace_back(tp.throwOrMove());
            }
            cb(AsyncResult());
    } );


    Parallel<int>::operation_t ops[] = {
        [&parallel5](Parallel<int>::callback_t cb)->void {
            parallel5.get().check();
            cb(OpResult<int>());
        },
        [&parallel25](Parallel<int>::callback_t cb)->void {
            parallel25.get().check();
            cb(OpResult<int>());
        }, 
        [](Parallel<int>::callback_t cb)->void {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            cb(OpResult<int>());
        },
        [](Parallel<int>::callback_t cb)->void {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            cb(OpResult<int>());
        },
        [&parallelTimes](Parallel<int>::callback_t cb)->void {
            parallelTimes.get().check();
            cb(OpResult<int>());
        }
    };

    auto result = Parallel<int>(manager, ops, 5).then( 
        [&times](OpResult<Parallel<int>::result_set_t>&&, Parallel<int>::complete_t cb)->void {
            times.emplace_back(std::chrono::high_resolution_clock::now());
            cb(AsyncResult());
    } );

    ASSERT_NO_THROW(result.get().throwIfError());

    for(size_t idx = 0; idx < nbTasks; ++idx)
    {
        ASSERT_LE(times[idx].time_since_epoch().count(), times[nbTasks].time_since_epoch().count());
    }

    manager->shutdown();
}