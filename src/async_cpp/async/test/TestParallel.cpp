#include "async_cpp/async/AsyncResult.h"
#include "async_cpp/async/Parallel.h"

#include "async_cpp/tasks/Manager.h"
#ifdef HAS_BOOST
#include "async_cpp/tasks/AsioManager.h"
#endif

#include <chrono>

#pragma warning(disable:4251)
#include <gtest/gtest.h>

using namespace async_cpp;
using namespace async_cpp::async;

TEST(PARALLEL_TEST, BASIC)
{
    auto manager(std::make_shared<tasks::Manager>(5));
    std::atomic<int> runCount(1);
    std::vector<int> taskRunOrder(6, 0);

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
                bool wasError = result.wasError();
                allSuccessful &= !wasError;
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

TEST(PARALLEL_TEST, TIMING)
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

    ASSERT_GE(totalDur, result->first);

    manager->shutdown();
}

#ifdef HAS_BOOST
TEST(PARALLEL_TEST, ASIO_TIMING)
{
    typedef std::chrono::high_resolution_clock::time_point data_t;
    typedef std::pair<std::chrono::high_resolution_clock::duration, bool> result_t;
    auto manager(std::make_shared<tasks::AsioManager>(5));

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

    ASSERT_GE(totalDur, result->first);

    manager->shutdown();
}
#endif