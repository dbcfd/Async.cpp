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
    std::vector<int> taskRunOrder(6, 0);

    std::function<AsyncFuture(void)> opsArray[] = {
        [&taskRunOrder, &runCount]()->AsyncFuture { 
            taskRunOrder[0] = runCount.fetch_add(1); 
            return AsyncResult().asFulfilledFuture();
        }, 
        [&taskRunOrder, &runCount]()->AsyncFuture { 
            taskRunOrder[1] = runCount.fetch_add(1); 
            return AsyncResult().asFulfilledFuture();
        },
        [&taskRunOrder, &runCount]()->AsyncFuture { 
            taskRunOrder[2] = runCount.fetch_add(1); 
            return AsyncResult().asFulfilledFuture();
        },
        [&taskRunOrder, &runCount]()->AsyncFuture { 
            taskRunOrder[3] = runCount.fetch_add(1); 
            return AsyncResult().asFulfilledFuture();
        },
        [&taskRunOrder, &runCount]()->AsyncFuture { 
            taskRunOrder[4] = runCount.fetch_add(1); 
            return AsyncResult().asFulfilledFuture();
        }
    };

    auto parallelResult = Parallel(manager, opsArray, 5).execute(
        [&taskRunOrder, &runCount](std::vector<AsyncResult>& results)->AsyncFuture {
            bool allSuccessful = true;
            for(auto& result : results)
            {
                allSuccessful &= result.wasError();
            }
            if(allSuccessful)
            {
                taskRunOrder[5] = runCount;
            }
            return AsyncResult().asFulfilledFuture();
        } );

    ASSERT_NO_THROW(parallelResult.get().throwIfError());

    for(int countNumber : taskRunOrder)
    {
        ASSERT_LE(5, runCount);
    }

    ASSERT_EQ(6, runCount);

    manager->shutdown();
}

TEST(ASYNC_TEST, PARALLEL_TIMING)
{
    auto manager(std::make_shared<tasks::Manager>(5));
    std::vector<std::unique_ptr<std::chrono::high_resolution_clock::time_point>> times(6);

    auto func = [&times](const size_t index)->AsyncFuture {
        times[index] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
            new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
            );
        return AsyncResult().asFulfilledFuture();
    };

    std::function<AsyncFuture(void)> opsArray[] = {
        std::bind(std::function<AsyncFuture(const size_t)>(func), 0), 
        std::bind(std::function<AsyncFuture(const size_t)>(func), 1), 
        std::bind(std::function<AsyncFuture(const size_t)>(func), 2), 
        std::bind(std::function<AsyncFuture(const size_t)>(func), 3), 
        std::bind(std::function<AsyncFuture(const size_t)>(func), 4) 
    };

    Parallel parallel(manager, opsArray, 5);
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    auto parallelResult = parallel.execute([&times, &start](std::vector<AsyncResult>& results)->AsyncFuture {
        bool wasSuccessful = true;
        for(auto& res : results)
        {
            wasSuccessful = !res.wasError();
        }
        if(wasSuccessful)
        {
            times[5] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
                new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
                );
        }
        return AsyncResult().asFulfilledFuture();
    } );

    ASSERT_NO_THROW(parallelResult.get().throwIfError());

    std::vector<std::chrono::high_resolution_clock::duration> durations;
    durations.reserve(times.size());

    for(std::unique_ptr<std::chrono::high_resolution_clock::time_point>& time : times)
    {
        durations.emplace_back((*time) - start);
    }

    long long maxMillis = 0;

    for(size_t idx = 0; idx < 5; ++idx)
    {
        EXPECT_LE(durations[idx].count(), durations[5].count());
        maxMillis = std::max(maxMillis, std::chrono::duration_cast<std::chrono::milliseconds>(durations[idx]).count());
    }

    ASSERT_LE(maxMillis, std::chrono::duration_cast<std::chrono::milliseconds>(durations[5]).count());

    manager->shutdown();
}

TEST(ASYNC_TEST, SERIES)
{
    auto manager(std::make_shared<tasks::Manager>(5));
    std::atomic<size_t> runCount(1);
    std::vector<size_t> runOrder(6, 0);

    std::function<AsyncFuture(AsyncResult&)> opsArray[] = {
        [&runOrder, &runCount](AsyncResult& res)->AsyncFuture {
            if(!res.wasError())
            {
                runOrder[0] = runCount.fetch_add(1);
            }
            return AsyncResult().asFulfilledFuture();
        },
        [&runOrder, &runCount](AsyncResult& res)->AsyncFuture {
            if(!res.wasError())
            {
                runOrder[1] = runCount.fetch_add(1);
            }
            return AsyncResult().asFulfilledFuture();
        },
        [&runOrder, &runCount](AsyncResult& res)->AsyncFuture {
            if(!res.wasError())
            {
                runOrder[2] = runCount.fetch_add(1);
            }
            return AsyncResult().asFulfilledFuture();
        },
        [&runOrder, &runCount](AsyncResult& res)->AsyncFuture {
            if(!res.wasError())
            {
                runOrder[3] = runCount.fetch_add(1);
            }
            return AsyncResult().asFulfilledFuture();
        },
        [&runOrder, &runCount](AsyncResult& res)->AsyncFuture {
            if(!res.wasError())
            {
                runOrder[4] = runCount.fetch_add(1);
            }
            return AsyncResult().asFulfilledFuture();
        }
    };

    auto seriesResult = Series(manager, opsArray, 5).execute(
        [&runOrder, &runCount](AsyncResult& result)->AsyncFuture {
            if(!result.wasError())
            {
                runOrder[5] = runCount;
            }
            return AsyncResult().asFulfilledFuture();
        } );

    ASSERT_NO_THROW(seriesResult.get().throwIfError());

    for(size_t idx = 0; idx < 6; ++idx)
    {
        ASSERT_EQ(idx+1, runOrder[idx]);
    }

    manager->shutdown();
}

TEST(ASYNC_TEST, SERIES_TIMING)
{
    auto manager(std::make_shared<tasks::Manager>(5));
    std::vector<std::unique_ptr<std::chrono::high_resolution_clock::time_point>> times(6);

    auto func = [&times](AsyncResult& res, const size_t index)->AsyncFuture {
        if(!res.wasError())
        {
            times[index] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
                new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
                );
        }
        return AsyncResult().asFulfilledFuture();
    };

    std::function<AsyncFuture(AsyncResult&)> opsArray[] = {
        std::bind(std::function<AsyncFuture(AsyncResult&, const size_t)>(func), std::placeholders::_1, 0), 
        std::bind(std::function<AsyncFuture(AsyncResult&, const size_t)>(func), std::placeholders::_1, 1), 
        std::bind(std::function<AsyncFuture(AsyncResult&, const size_t)>(func), std::placeholders::_1, 2), 
        std::bind(std::function<AsyncFuture(AsyncResult&, const size_t)>(func), std::placeholders::_1, 3), 
        std::bind(std::function<AsyncFuture(AsyncResult&, const size_t)>(func), std::placeholders::_1, 4) 
    };

    Series series(manager, opsArray, 5);
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    auto seriesResult = series.execute([&times, &start](AsyncResult& result)->AsyncFuture {
        if(!result.wasError())
        {
            times[5] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
                new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
                );
        }
        return AsyncResult().asFulfilledFuture();
    } );

    ASSERT_NO_THROW(seriesResult.get().throwIfError());

    std::chrono::high_resolution_clock::time_point last = (*times[0]);

    for(size_t idx = 1; idx < 5; ++idx)
    {
        std::chrono::high_resolution_clock::time_point current = (*times[idx]);
        ASSERT_LE(last, current);
        last = current;
    }

    ASSERT_LE(last, (*times[5]));

    manager->shutdown();
}

TEST(ASYNC_TEST, PARALLEL_FOREACH)
{
    auto manager(std::make_shared<tasks::Manager>(5));
    std::vector<std::unique_ptr<std::chrono::high_resolution_clock::time_point>> times(6);

    auto func = [&times](std::shared_ptr<const void> data)->AsyncFuture {
        std::shared_ptr<const int> index = std::static_pointer_cast<const int>(data);
        times[*index] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
            new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
            );
        return AsyncResult().asFulfilledFuture();
    };

    std::vector<std::shared_ptr<void>> data;
    data.emplace_back(std::make_shared<int>(0));
    data.emplace_back(std::make_shared<int>(1));
    data.emplace_back(std::make_shared<int>(2));
    data.emplace_back(std::make_shared<int>(3));
    data.emplace_back(std::make_shared<int>(4));

    ParallelForEach parallel(manager, func, data);
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    auto parallelResult = parallel.execute([&times, &start](std::vector<AsyncResult>& results)->AsyncFuture {
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
        return AsyncResult().asFulfilledFuture();
    } );

    ASSERT_NO_THROW(parallelResult.get().throwIfError());

    std::vector<std::chrono::high_resolution_clock::duration> durations;
    durations.reserve(times.size());

    for(std::unique_ptr<std::chrono::high_resolution_clock::time_point>& time : times)
    {
        durations.emplace_back((*time) - start);
    }

    long long maxMillis = 0;

    for(size_t idx = 0; idx < 5; ++idx)
    {
        EXPECT_LE(durations[idx].count(), durations[5].count());
        maxMillis = std::max(maxMillis, std::chrono::duration_cast<std::chrono::milliseconds>(durations[idx]).count());
    }

    ASSERT_LE(maxMillis, std::chrono::duration_cast<std::chrono::milliseconds>(durations[5]).count());

    manager->shutdown();
}

TEST(ASYNC_TEST, PARALLEL_FOR)
{
    size_t nbTasks = 5;
    auto manager(std::make_shared<tasks::Manager>(nbTasks));
    std::vector<std::unique_ptr<std::chrono::high_resolution_clock::time_point>> times(nbTasks+1);

    auto func = [&times](size_t index)->AsyncFuture {
        times[index] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
            new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
            );
        return AsyncResult().asFulfilledFuture();
    };

    ParallelFor parallel(manager, func, nbTasks);
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    auto parallelResult = parallel.execute([&times, &start, nbTasks](std::vector<AsyncResult>& results)->AsyncFuture {
        bool wasSuccesful = true;
        for(auto& res : results)
        {
            wasSuccesful &= !res.wasError();
        }
        if(wasSuccesful)
        {
            times[nbTasks] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
                new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
                );
        }
        return AsyncResult().asFulfilledFuture();
    } );

    ASSERT_NO_THROW(parallelResult.get().throwIfError());

    std::vector<std::chrono::high_resolution_clock::duration> durations;
    durations.reserve(times.size());

    for(std::unique_ptr<std::chrono::high_resolution_clock::time_point>& time : times)
    {
        durations.emplace_back((*time) - start);
    }

    long long maxMillis = 0;

    for(size_t idx = 0; idx < nbTasks; ++idx)
    {
        EXPECT_LE(durations[idx].count(), durations[nbTasks].count());
        maxMillis = std::max(maxMillis, std::chrono::duration_cast<std::chrono::milliseconds>(durations[idx]).count());
    }

    ASSERT_LE(maxMillis, std::chrono::duration_cast<std::chrono::milliseconds>(durations[nbTasks]).count());

    manager->shutdown();
}

TEST(ASYNC_TEST, OVERLOAD_MANAGER)
{
    size_t nbTasks = 5;
    auto manager(std::make_shared<tasks::Manager>(3));
    std::vector<std::unique_ptr<std::chrono::high_resolution_clock::time_point>> times(nbTasks+1);

    auto func = [&times](size_t index)->AsyncFuture {
        times[index] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
            new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
            );
        return AsyncResult().asFulfilledFuture();
    };

    auto parallel5 = ParallelFor(manager,
        [](size_t index)->AsyncFuture {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    return AsyncResult().asFulfilledFuture();
        }, 5).execute();

    auto parallel25 = ParallelFor(manager,
        [](size_t index)->AsyncFuture {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            return AsyncResult().asFulfilledFuture();
        }, 25).execute();

    auto parallelTimes = ParallelFor(manager, func, nbTasks).execute(
        [&times](std::vector<AsyncResult>& results)->AsyncFuture {
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
            return AsyncResult().asFulfilledFuture();
        } );


    std::function<AsyncFuture(AsyncResult&)> ops[] = {
        [&parallel5](AsyncResult&)->AsyncFuture {
            return std::move(parallel5);
        },
        [&parallel25](AsyncResult&)->AsyncFuture {
            return std::move(parallel25);
        }, 
        [](AsyncResult&)->AsyncFuture {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            return AsyncResult().asFulfilledFuture();
        },
        [](AsyncResult&)->AsyncFuture {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            return AsyncResult().asFulfilledFuture();
        },
        [&parallelTimes](AsyncResult&)->AsyncFuture {
            return std::move(parallelTimes);
        }
    };

    auto result = Series(manager, ops, 5).execute();

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

    auto func = [&times](size_t index)->AsyncFuture {
        times[index] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
            new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
            );
        return AsyncResult().asFulfilledFuture();
    };

    auto parallel5 = ParallelFor(manager,
        [](size_t index)->AsyncFuture {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    return AsyncResult().asFulfilledFuture();
        }, 5).execute();

    auto parallel25 = ParallelFor(manager,
        [](size_t index)->AsyncFuture {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            return AsyncResult().asFulfilledFuture();
        }, 25).execute();

    auto parallelTimes = ParallelFor(manager, func, nbTasks).execute(
        [&times](std::vector<AsyncResult>& results)->AsyncFuture {
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
            return AsyncResult().asFulfilledFuture();
        } );


    std::function<AsyncFuture()> ops[] = {
        [&parallel5]()->AsyncFuture {
            return std::move(parallel5);
        },
        [&parallel25]()->AsyncFuture {
            return std::move(parallel25);
        }, 
        []()->AsyncFuture {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            return AsyncResult().asFulfilledFuture();
        },
        []()->AsyncFuture {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            return AsyncResult().asFulfilledFuture();
        },
        [&parallelTimes]()->AsyncFuture {
            return std::move(parallelTimes);
        }
    };

    auto result = Parallel(manager, ops, 5).execute();

    ASSERT_NO_THROW(result.get().throwIfError());

    for(size_t idx = 0; idx < nbTasks; ++idx)
    {
        ASSERT_LE(times[idx]->time_since_epoch().count(), times[nbTasks]->time_since_epoch().count());
    }

    manager->shutdown();
}