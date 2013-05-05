#include "async_cpp/async/AsyncResult.h"
#include "async_cpp/async/Parallel.h"
#include "async_cpp/async/ParallelFor.h"
#include "async_cpp/async/ParallelForEach.h"
#include "async_cpp/async/Series.h"

#include "async_cpp/tasks/Manager.h"

#include <chrono>

#pragma warning(disable:4251)
#include <gtest/gtest.h>

using namespace async_cpp;
using namespace async_cpp::async;

TEST(ASYNC_TEST, PARALLEL)
{
    auto manager(std::make_shared<tasks::Manager>(5));
    std::atomic<int> runCount(1);
    std::vector<int> taskRunOrder(5, 0);

    std::function<std::future<AsyncResult<void>>(void)> opsArray[] = {
        [&taskRunOrder, &runCount]()->std::future<AsyncResult<void>> { 
            taskRunOrder[0] = runCount.fetch_add(1); 
            return AsyncResult<void>().asFulfilledFuture();
        }, 
        [&taskRunOrder, &runCount]()->std::future<AsyncResult<void>> { 
            taskRunOrder[1] = runCount.fetch_add(1); 
            return AsyncResult<void>().asFulfilledFuture();
        },
        [&taskRunOrder, &runCount]()->std::future<AsyncResult<void>> { 
            taskRunOrder[2] = runCount.fetch_add(1); 
            return AsyncResult<void>().asFulfilledFuture();
        },
        [&taskRunOrder, &runCount]()->std::future<AsyncResult<void>> { 
            taskRunOrder[3] = runCount.fetch_add(1); 
            return AsyncResult<void>().asFulfilledFuture();
        },
        [&taskRunOrder, &runCount]()->std::future<AsyncResult<void>> { 
            taskRunOrder[4] = runCount.fetch_add(1); 
            return AsyncResult<void>().asFulfilledFuture();
        }
    };

    auto parallelResult = Parallel<void,bool>(manager, opsArray, 5).execute(
        [&taskRunOrder, &runCount](const std::vector<AsyncResult<void>>& results)->std::future<AsyncResult<bool>> {
            bool allSuccessful = true;
            for(auto& result : results)
            {
                allSuccessful &= result.wasError();
            }
            if(allSuccessful)
            {
                taskRunOrder[5] = runCount;
            }
            return AsyncResult<bool>(std::make_shared<bool>(allSuccessful)).asFulfilledFuture();
        } );

    AsyncResult<bool> result;
    ASSERT_NO_THROW(result = parallelResult.get());
    std::shared_ptr<bool> wasSuccessful;
    ASSERT_NO_THROW(wasSuccessful = result.throwOrGet());

    for(int countNumber : taskRunOrder)
    {
        ASSERT_LE(5, runCount);
    }

    ASSERT_TRUE(wasSuccessful && *wasSuccessful);

    manager->shutdown();
}

TEST(ASYNC_TEST, PARALLEL_TIMING)
{
    typedef std::chrono::high_resolution_clock::time_point data_t;
    typedef std::pair<std::chrono::high_resolution_clock::duration, bool> result_t;
    auto manager(std::make_shared<tasks::Manager>(5));

    auto func = []()->std::future<AsyncResult<data_t>> {
        return AsyncResult<data_t>(std::make_shared<data_t>(std::chrono::high_resolution_clock::now())).asFulfilledFuture();
    };

    auto ops = std::vector<std::function<std::future<AsyncResult<data_t>>(void)>>(5, func);

    Parallel<data_t, result_t> parallel(manager, ops);
    auto start = std::chrono::high_resolution_clock::now();
    auto future = parallel.execute([&start](const std::vector<AsyncResult<data_t>>& results)->std::future<AsyncResult<result_t>> {
        bool wasSuccessful = true;
        auto maxDur = std::chrono::high_resolution_clock::duration::min();
        for(auto& res : results)
        {
            try
            {
                auto taskExecute = res.throwOrGet();
                wasSuccessful = true;
                maxDur = std::max(maxDur, *taskExecute - start);
            }
            catch(std::runtime_error&)
            {
                wasSuccessful = false;
                break;
            }
        }
        return AsyncResult<result_t>(std::make_shared<result_t>(std::make_pair(maxDur, wasSuccessful))).asFulfilledFuture();
    } );
    AsyncResult<result_t> parallelResult;
    ASSERT_NO_THROW(parallelResult = future.get());
    auto totalDur = std::chrono::high_resolution_clock::now() - start;

    std::shared_ptr<result_t> result;
    ASSERT_NO_THROW(result = parallelResult.throwOrGet());
    ASSERT_TRUE(result->second);

    ASSERT_LE(totalDur, result->first);

    manager->shutdown();
}

TEST(ASYNC_TEST, SERIES)
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

TEST(ASYNC_TEST, SERIES_TIMING)
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

TEST(ASYNC_TEST, PARALLEL_FOREACH)
{
    typedef std::chrono::high_resolution_clock::duration result_t;
    auto manager(std::make_shared<tasks::Manager>(5));
    std::vector<std::shared_ptr<std::chrono::high_resolution_clock::time_point>> times(6);

    auto func = [&times](std::shared_ptr<size_t> index)->std::future<AsyncResult<size_t>> {
        times[*index] = std::make_shared<std::chrono::high_resolution_clock::time_point>(std::chrono::high_resolution_clock::now());
        return AsyncResult<size_t>(index).asFulfilledFuture();
    };

    std::vector<std::shared_ptr<size_t>> data;
    data.emplace_back(std::make_shared<size_t>(0));
    data.emplace_back(std::make_shared<size_t>(1));
    data.emplace_back(std::make_shared<size_t>(2));
    data.emplace_back(std::make_shared<size_t>(3));
    data.emplace_back(std::make_shared<size_t>(4));

    ParallelForEach<size_t, size_t, result_t> parallel(manager, func, data);
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

TEST(ASYNC_TEST, PARALLEL_FOR)
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

TEST(ASYNC_TEST, OVERLOAD_MANAGER)
{
    size_t nbTasks = 5;
    auto manager(std::make_shared<tasks::Manager>(3));
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
        [&times](std::vector<AsyncResult<void>>& results)->std::future<AsyncResult<void>> {
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

TEST(ASYNC_TEST, OVERLOAD_MANAGER_PARALLEL)
{
    size_t nbTasks = 5;
    auto manager(std::make_shared<tasks::Manager>(3));
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
        [&times](std::vector<AsyncResult<void>>& results)->std::future<AsyncResult<void>> {
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