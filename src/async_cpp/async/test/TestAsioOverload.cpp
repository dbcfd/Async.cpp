#ifdef HAS_BOOST
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

TEST(ASIO_OVERLOAD_TEST, SERIES)
{
    size_t nbTasks = 5;
    auto manager(std::make_shared<tasks::AsioManager>(3));
    std::vector<std::unique_ptr<std::chrono::high_resolution_clock::time_point>> times(nbTasks+1);

    auto func = [&times](size_t index)->std::future<AsyncResult<void>> {
        times[index] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
            new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
            );
        return AsyncResult<void>().asFulfilledFuture();
    };

    auto parallel5 = ParallelFor<void, void>(manager,
        [](size_t index)->std::future<AsyncResult<void>> {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    return AsyncResult<void>().asFulfilledFuture();
        }, 5).execute();

    auto parallel25 = ParallelFor<void, void>(manager,
        [](size_t index)->std::future<AsyncResult<void>> {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            return AsyncResult<void>().asFulfilledFuture();
        }, 25).execute();

    auto parallelTimes = ParallelFor<void, void>(manager, func, nbTasks).execute(
        [&times](const std::vector<AsyncResult<void>>& results)->std::future<AsyncResult<void>> {
            bool wasSuccessful = true;
            for(auto& res : results)
            {
                wasSuccessful &= !res.wasError();
            }
            if(wasSuccessful)
            {
                times[5] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
                    new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
                    );
            }
            return AsyncResult<void>().asFulfilledFuture();
        } );


    std::function<std::future<AsyncResult<void>>(const AsyncResult<void>&)> ops[] = {
        [&parallel5](const AsyncResult<void>&)->std::future<AsyncResult<void>> {
            return std::move(parallel5);
        },
        [&parallel25](const AsyncResult<void>&)->std::future<AsyncResult<void>> {
            return std::move(parallel25);
        }, 
        [](const AsyncResult<void>&)->std::future<AsyncResult<void>> {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            return AsyncResult<void>().asFulfilledFuture();
        },
        [](const AsyncResult<void>&)->std::future<AsyncResult<void>> {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            return AsyncResult<void>().asFulfilledFuture();
        },
        [&parallelTimes](const AsyncResult<void>&)->std::future<AsyncResult<void>> {
            return std::move(parallelTimes);
        }
    };

    auto result = Series<void, void>(manager, ops, 5).execute();

    ASSERT_NO_THROW(result.get().throwIfError());

    for(size_t idx = 0; idx < nbTasks; ++idx)
    {
        ASSERT_LE(times[idx]->time_since_epoch().count(), times[nbTasks]->time_since_epoch().count());
    }

    manager->shutdown();
}

TEST(ASIO_OVERLOAD_TEST, PARALLEL)
{
    size_t nbTasks = 5;
    auto manager(std::make_shared<tasks::AsioManager>(3));
    std::vector<std::unique_ptr<std::chrono::high_resolution_clock::time_point>> times(nbTasks+1);

    auto func = [&times](size_t index)->std::future<AsyncResult<void>> {
        times[index] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
            new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
            );
        return AsyncResult<void>().asFulfilledFuture();
    };

    auto parallel5 = ParallelFor<void, void>(manager,
        [](size_t index)->std::future<AsyncResult<void>> {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    return AsyncResult<void>().asFulfilledFuture();
        }, 5).execute();

    auto parallel25 = ParallelFor<void, void>(manager,
        [](size_t index)->std::future<AsyncResult<void>> {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            return AsyncResult<void>().asFulfilledFuture();
        }, 25).execute();

    auto parallelTimes = ParallelFor<void, void>(manager, func, nbTasks).execute(
        [&times](const std::vector<AsyncResult<void>>& results)->std::future<AsyncResult<void>> {
            bool wasSuccessful = true;
            for(auto& res : results)
            {
                wasSuccessful &= !res.wasError();
            }
            if(wasSuccessful)
            {
                times[5] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
                    new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
                    );
            }
            return AsyncResult<void>().asFulfilledFuture();
        } );


    std::function<std::future<AsyncResult<void>>()> ops[] = {
        [&parallel5]()->std::future<AsyncResult<void>> {
            return std::move(parallel5);
        },
        [&parallel25]()->std::future<AsyncResult<void>> {
            return std::move(parallel25);
        }, 
        []()->std::future<AsyncResult<void>> {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            return AsyncResult<void>().asFulfilledFuture();
        },
        []()->std::future<AsyncResult<void>> {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            return AsyncResult<void>().asFulfilledFuture();
        },
        [&parallelTimes]()->std::future<AsyncResult<void>> {
            return std::move(parallelTimes);
        }
    };

    auto result = Parallel<void, void>(manager, ops, 5).execute();

    ASSERT_NO_THROW(result.get().throwIfError());

    for(size_t idx = 0; idx < nbTasks; ++idx)
    {
        ASSERT_LE(times[idx]->time_since_epoch().count(), times[nbTasks]->time_since_epoch().count());
    }

    manager->shutdown();
}

#endif