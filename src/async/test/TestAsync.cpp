#include "async/AsyncResult.h"
#include "async/Parallel.h"
#include "async/ParallelForEach.h"
#include "async/Series.h"

#include "workers/Manager.h"

#include <chrono>

#pragma warning(disable:4251)
#include <gtest/gtest.h>

using namespace quicktcp;

TEST(ASYNC_TEST, PARALLEL)
{
    std::shared_ptr<workers::Manager> manager(new workers::Manager(5));
    std::atomic<int> runCount(1);
    std::vector<int> taskRunOrder(6, 0);

    std::function<async::PtrAsyncResult(void)> opsArray[] = {
        [&taskRunOrder, &runCount]()->async::PtrAsyncResult { 
            taskRunOrder[0] = runCount.fetch_add(1); 
            return async::PtrAsyncResult(new async::AsyncResult()); 
        }, 
        [&taskRunOrder, &runCount]()->async::PtrAsyncResult { 
            taskRunOrder[1] = runCount.fetch_add(1); 
            return async::PtrAsyncResult(new async::AsyncResult()); 
        },
        [&taskRunOrder, &runCount]()->async::PtrAsyncResult { 
            taskRunOrder[2] = runCount.fetch_add(1); 
            return async::PtrAsyncResult(new async::AsyncResult()); 
        },
        [&taskRunOrder, &runCount]()->async::PtrAsyncResult { 
            taskRunOrder[3] = runCount.fetch_add(1); 
            return async::PtrAsyncResult(new async::AsyncResult()); 
        },
        [&taskRunOrder, &runCount]()->async::PtrAsyncResult { 
            taskRunOrder[4] = runCount.fetch_add(1); 
            return async::PtrAsyncResult(new async::AsyncResult()); 
        }
    };

    std::future<async::PtrAsyncResult> parallelFuture = async::Parallel(manager, opsArray, 5).execute(
        [&taskRunOrder, &runCount](async::PtrAsyncResult result)->async::PtrAsyncResult {
            result->throwIfError();
            taskRunOrder[5] = runCount;
            return result;
        } );

    async::PtrAsyncResult parallelResult = parallelFuture.get();

    ASSERT_NO_THROW(parallelResult->throwIfError());

    for(int countNumber : taskRunOrder)
    {
        ASSERT_LE(5, runCount);
    }

    ASSERT_EQ(6, runCount);
}

TEST(ASYNC_TEST, PARALLEL_TIMING)
{
    std::shared_ptr<workers::Manager> manager(new workers::Manager(5));
    std::vector<std::unique_ptr<std::chrono::high_resolution_clock::time_point>> times(6);

    auto func = [&times](const size_t index)->async::PtrAsyncResult {
        times[index] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
            new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
            );
        return async::PtrAsyncResult(new async::AsyncResult());
    };

    std::function<async::PtrAsyncResult(void)> opsArray[] = {
        std::bind(std::function<async::PtrAsyncResult(const size_t)>(func), 0), 
        std::bind(std::function<async::PtrAsyncResult(const size_t)>(func), 1), 
        std::bind(std::function<async::PtrAsyncResult(const size_t)>(func), 2), 
        std::bind(std::function<async::PtrAsyncResult(const size_t)>(func), 3), 
        std::bind(std::function<async::PtrAsyncResult(const size_t)>(func), 4) 
    };

    async::Parallel parallel(manager, opsArray, 5);
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    std::future<async::PtrAsyncResult> parallelFuture = parallel.execute([&times, &start](async::PtrAsyncResult result)->async::PtrAsyncResult {
        times[5] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
            new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
            );
        return result;
    } );

    async::PtrAsyncResult result = parallelFuture.get();

    ASSERT_NO_THROW(result->throwIfError());

    std::vector<std::chrono::high_resolution_clock::duration> durations;
    durations.reserve(times.size());

    for(std::unique_ptr<std::chrono::high_resolution_clock::time_point>& time : times)
    {
        durations.emplace_back((*time) - start);
    }

    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(durations[0]).count();
    auto maxMillis = millis;

    for(size_t idx = 1; idx < 5; ++idx)
    {
        auto curMillis = std::chrono::duration_cast<std::chrono::milliseconds>(durations[idx]).count();
        ASSERT_GE(1, abs(millis - curMillis));
        maxMillis = std::max(curMillis, maxMillis);
    }

    ASSERT_LE(maxMillis, std::chrono::duration_cast<std::chrono::milliseconds>(durations[5]).count());
}

TEST(ASYNC_TEST, SERIES)
{
    std::shared_ptr<workers::Manager> manager(new workers::Manager(5));
    std::atomic<size_t> runCount(1);
    std::vector<size_t> runOrder(6, 0);

    std::function<async::PtrAsyncResult(async::PtrAsyncResult)> opsArray[] = {
        [&runOrder, &runCount](async::PtrAsyncResult res)->async::PtrAsyncResult {
            runOrder[0] = runCount.fetch_add(1);
            return res;
        },
        [&runOrder, &runCount](async::PtrAsyncResult res)->async::PtrAsyncResult {
            runOrder[1] = runCount.fetch_add(1);
            return res;
        },
        [&runOrder, &runCount](async::PtrAsyncResult res)->async::PtrAsyncResult {
            runOrder[2] = runCount.fetch_add(1);
            return res;
        },
        [&runOrder, &runCount](async::PtrAsyncResult res)->async::PtrAsyncResult {
            runOrder[3] = runCount.fetch_add(1);
            return res;
        },
        [&runOrder, &runCount](async::PtrAsyncResult res)->async::PtrAsyncResult {
            runOrder[4] = runCount.fetch_add(1);
            return res;
        }
    };

    std::future<async::PtrAsyncResult> seriesFuture = async::Series(manager, opsArray, 5).execute(
        [&runOrder, &runCount](async::PtrAsyncResult result)->async::PtrAsyncResult {
            runOrder[5] = runCount;
            return result;
        } );

    async::PtrAsyncResult result = seriesFuture.get();

    ASSERT_NO_THROW(result->throwIfError());

    for(size_t idx = 0; idx < 6; ++idx)
    {
        ASSERT_EQ(idx+1, runOrder[idx]);
    }
}

TEST(ASYNC_TEST, SERIES_TIMING)
{
    std::shared_ptr<workers::Manager> manager(new workers::Manager(5));
    std::vector<std::unique_ptr<std::chrono::high_resolution_clock::time_point>> times(6);

    auto func = [&times](async::PtrAsyncResult res, const size_t index)->async::PtrAsyncResult {
        times[index] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
            new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
            );
        return res;
    };

    std::function<async::PtrAsyncResult(async::PtrAsyncResult)> opsArray[] = {
        std::bind(std::function<async::PtrAsyncResult(async::PtrAsyncResult, const size_t)>(func), std::placeholders::_1, 0), 
        std::bind(std::function<async::PtrAsyncResult(async::PtrAsyncResult, const size_t)>(func), std::placeholders::_1, 1), 
        std::bind(std::function<async::PtrAsyncResult(async::PtrAsyncResult, const size_t)>(func), std::placeholders::_1, 2), 
        std::bind(std::function<async::PtrAsyncResult(async::PtrAsyncResult, const size_t)>(func), std::placeholders::_1, 3), 
        std::bind(std::function<async::PtrAsyncResult(async::PtrAsyncResult, const size_t)>(func), std::placeholders::_1, 4) 
    };

    async::Series series(manager, opsArray, 5);
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    std::future<async::PtrAsyncResult> seriesFuture = series.execute([&times, &start](async::PtrAsyncResult result)->async::PtrAsyncResult {
        times[5] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
            new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
            );
        return result;
    } );

    async::PtrAsyncResult result = seriesFuture.get();

    ASSERT_NO_THROW(result->throwIfError());

    std::chrono::high_resolution_clock::time_point last = (*times[0]);

    for(size_t idx = 1; idx < 5; ++idx)
    {
        std::chrono::high_resolution_clock::time_point current = (*times[idx]);
        ASSERT_LE(last, current);
        last = current;
    }

    ASSERT_LE(last, (*times[5]));
}

TEST(ASYNC_TEST, PARALLEL_FOREACH)
{
    std::shared_ptr<workers::Manager> manager(new workers::Manager(5));
    std::vector<std::unique_ptr<std::chrono::high_resolution_clock::time_point>> times(6);

    auto func = [&times](std::shared_ptr<void> data)->async::PtrAsyncResult {
        std::shared_ptr<int> index = std::static_pointer_cast<int>(data);
        times[*index] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
            new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
            );
        return async::PtrAsyncResult(new async::AsyncResult());
    };

    std::vector<std::shared_ptr<void>> data;
    data.emplace_back(new int(0));
    data.emplace_back(new int(1));
    data.emplace_back(new int(2));
    data.emplace_back(new int(3));
    data.emplace_back(new int(4));

    async::ParallelForEach parallel(manager, func, data);
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    std::future<async::PtrAsyncResult> parallelFuture = parallel.execute([&times, &start](const std::vector<async::PtrAsyncResult>& results)->async::PtrAsyncResult {
        times[5] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
            new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
            );
        return async::PtrAsyncResult(new async::AsyncResult());
    } );

    async::PtrAsyncResult result = parallelFuture.get();

    ASSERT_NO_THROW(result->throwIfError());

    std::vector<std::chrono::high_resolution_clock::duration> durations;
    durations.reserve(times.size());

    for(std::unique_ptr<std::chrono::high_resolution_clock::time_point>& time : times)
    {
        durations.emplace_back((*time) - start);
    }

    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(durations[0]).count();
    auto maxMillis = millis;

    for(size_t idx = 1; idx < 5; ++idx)
    {
        auto curMillis = std::chrono::duration_cast<std::chrono::milliseconds>(durations[idx]).count();
        ASSERT_GE(1, abs(millis - curMillis));
        maxMillis = std::max(curMillis, maxMillis);
    }

    ASSERT_LE(maxMillis, std::chrono::duration_cast<std::chrono::milliseconds>(durations[5]).count());
}