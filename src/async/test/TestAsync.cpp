#include "async/AsyncResult.h"
#include "async/Parallel.h"
#include "async/ParallelFor.h"
#include "async/ParallelForEach.h"
#include "async/Series.h"

#include "workers/Manager.h"

#include <chrono>

#pragma warning(disable:4251)
#include <gtest/gtest.h>

using namespace async_cpp;
using namespace async_cpp::async;

TEST(ASYNC_TEST, PARALLEL)
{
    std::shared_ptr<workers::Manager> manager(new workers::Manager(5));
    std::atomic<int> runCount(1);
    std::vector<int> taskRunOrder(6, 0);

    std::function<AsyncResult(void)> opsArray[] = {
        [&taskRunOrder, &runCount]()->AsyncResult { 
            taskRunOrder[0] = runCount.fetch_add(1); 
            return AsyncResult();
        }, 
        [&taskRunOrder, &runCount]()->AsyncResult { 
            taskRunOrder[1] = runCount.fetch_add(1); 
            return AsyncResult();
        },
        [&taskRunOrder, &runCount]()->AsyncResult { 
            taskRunOrder[2] = runCount.fetch_add(1); 
            return AsyncResult();
        },
        [&taskRunOrder, &runCount]()->AsyncResult { 
            taskRunOrder[3] = runCount.fetch_add(1); 
            return AsyncResult();
        },
        [&taskRunOrder, &runCount]()->AsyncResult { 
            taskRunOrder[4] = runCount.fetch_add(1); 
            return AsyncResult();
        }
    };

    auto parallelResult = Parallel(manager, opsArray, 5).execute(
        [&taskRunOrder, &runCount](AsyncResult& result)->AsyncResult {
            if(!result.wasError())
            {
                taskRunOrder[5] = runCount;
            }
            return std::move(result);
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
    std::shared_ptr<workers::Manager> manager(new workers::Manager(5));
    std::vector<std::unique_ptr<std::chrono::high_resolution_clock::time_point>> times(6);

    auto func = [&times](const size_t index)->AsyncResult {
        times[index] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
            new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
            );
        return AsyncResult();
    };

    std::function<AsyncResult(void)> opsArray[] = {
        std::bind(std::function<AsyncResult(const size_t)>(func), 0), 
        std::bind(std::function<AsyncResult(const size_t)>(func), 1), 
        std::bind(std::function<AsyncResult(const size_t)>(func), 2), 
        std::bind(std::function<AsyncResult(const size_t)>(func), 3), 
        std::bind(std::function<AsyncResult(const size_t)>(func), 4) 
    };

    Parallel parallel(manager, opsArray, 5);
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    auto parallelResult = parallel.execute([&times, &start](AsyncResult& result)->AsyncResult {
        if(!result.wasError())
        {
            times[5] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
                new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
                );
        }
        return std::move(result);
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
    std::shared_ptr<workers::Manager> manager(new workers::Manager(5));
    std::atomic<size_t> runCount(1);
    std::vector<size_t> runOrder(6, 0);

    std::function<AsyncResult(AsyncResult&)> opsArray[] = {
        [&runOrder, &runCount](AsyncResult& res)->AsyncResult {
            if(!res.wasError())
            {
                runOrder[0] = runCount.fetch_add(1);
            }
            return std::move(res);
        },
        [&runOrder, &runCount](AsyncResult& res)->AsyncResult {
            if(!res.wasError())
            {
                runOrder[1] = runCount.fetch_add(1);
            }
            return std::move(res);
        },
        [&runOrder, &runCount](AsyncResult& res)->AsyncResult {
            if(!res.wasError())
            {
                runOrder[2] = runCount.fetch_add(1);
            }
            return std::move(res);
        },
        [&runOrder, &runCount](AsyncResult& res)->AsyncResult {
            if(!res.wasError())
            {
                runOrder[3] = runCount.fetch_add(1);
            }
            return std::move(res);
        },
        [&runOrder, &runCount](AsyncResult& res)->AsyncResult {
            if(!res.wasError())
            {
                runOrder[4] = runCount.fetch_add(1);
            }
            return std::move(res);
        }
    };

    auto seriesResult = Series(manager, opsArray, 5).execute(
        [&runOrder, &runCount](AsyncResult& result)->AsyncResult {
            if(!result.wasError())
            {
                runOrder[5] = runCount;
            }
            return std::move(result);
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
    std::shared_ptr<workers::IManager> manager(new workers::Manager(5));
    std::vector<std::unique_ptr<std::chrono::high_resolution_clock::time_point>> times(6);

    auto func = [&times](AsyncResult& res, const size_t index)->AsyncResult {
        if(!res.wasError())
        {
            times[index] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
                new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
                );
        }
        return std::move(res);
    };

    std::function<AsyncResult(AsyncResult&)> opsArray[] = {
        std::bind(std::function<AsyncResult(AsyncResult&, const size_t)>(func), std::placeholders::_1, 0), 
        std::bind(std::function<AsyncResult(AsyncResult&, const size_t)>(func), std::placeholders::_1, 1), 
        std::bind(std::function<AsyncResult(AsyncResult&, const size_t)>(func), std::placeholders::_1, 2), 
        std::bind(std::function<AsyncResult(AsyncResult&, const size_t)>(func), std::placeholders::_1, 3), 
        std::bind(std::function<AsyncResult(AsyncResult&, const size_t)>(func), std::placeholders::_1, 4) 
    };

    Series series(manager, opsArray, 5);
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    auto seriesResult = series.execute([&times, &start](AsyncResult& result)->AsyncResult {
        if(!result.wasError())
        {
            times[5] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
                new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
                );
        }
        return std::move(result);
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
    std::shared_ptr<workers::Manager> manager(new workers::Manager(5));
    std::vector<std::unique_ptr<std::chrono::high_resolution_clock::time_point>> times(6);

    auto func = [&times](std::shared_ptr<void> data)->AsyncResult {
        std::shared_ptr<int> index = std::static_pointer_cast<int>(data);
        times[*index] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
            new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
            );
        return AsyncResult();
    };

    std::vector<std::shared_ptr<void>> data;
    data.emplace_back(new int(0));
    data.emplace_back(new int(1));
    data.emplace_back(new int(2));
    data.emplace_back(new int(3));
    data.emplace_back(new int(4));

    ParallelForEach parallel(manager, func, data);
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    auto parallelResult = parallel.execute([&times, &start](AsyncResult& result)->AsyncResult {
        if(!result.wasError())
        {
            times[5] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
                new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
                );
        }
        return std::move(result);
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
    std::shared_ptr<workers::Manager> manager(new workers::Manager(nbTasks));
    std::vector<std::unique_ptr<std::chrono::high_resolution_clock::time_point>> times(nbTasks+1);

    auto func = [&times](std::shared_ptr<void> data)->AsyncResult {
        auto index = std::static_pointer_cast<size_t>(data);
        times[*index] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
            new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
            );
        return AsyncResult();
    };

    ParallelFor parallel(manager, func, nbTasks);
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    auto parallelResult = parallel.execute([&times, &start, nbTasks](AsyncResult& result)->AsyncResult {
        if(!result.wasError())
        {
            times[nbTasks] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
                new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
                );
        }
        return std::move(result);
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
    std::shared_ptr<workers::Manager> manager(new workers::Manager(3));
    std::vector<std::unique_ptr<std::chrono::high_resolution_clock::time_point>> times(nbTasks+1);

    auto func = [&times](std::shared_ptr<void> data)->AsyncResult {
        auto index = std::static_pointer_cast<size_t>(data);
        times[*index] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
            new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
            );
        return AsyncResult();
    };

    auto parallel5 = ParallelFor(manager,
        [](std::shared_ptr<void> data)->AsyncResult {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    return AsyncResult();
        }, 5).execute();

    auto parallel25 = ParallelFor(manager,
        [](std::shared_ptr<void> data)->AsyncResult {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            return AsyncResult();
        }, 25).execute();

    auto parallelTimes = ParallelFor(manager, func, nbTasks).execute(
        [&times](AsyncResult& res)->AsyncResult {
            if(!res.wasError())
            {
                times[5] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
                    new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
                    );
            }
            return std::move(res);
        } );


    std::function<AsyncResult(AsyncResult&)> ops[] = {
        [&parallel5](AsyncResult&)->AsyncResult {
            return parallel5.get();
        },
        [&parallel25](AsyncResult&)->AsyncResult {
            return parallel25.get();
        }, 
        [](AsyncResult&)->AsyncResult {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            return AsyncResult();
        },
        [](AsyncResult&)->AsyncResult {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            return AsyncResult();
        },
        [&parallelTimes](AsyncResult&)->AsyncResult {
            return parallelTimes.get();
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
    std::shared_ptr<workers::Manager> manager(new workers::Manager(3));
    std::vector<std::unique_ptr<std::chrono::high_resolution_clock::time_point>> times(nbTasks+1);

    auto func = [&times](std::shared_ptr<void> data)->AsyncResult {
        auto index = std::static_pointer_cast<size_t>(data);
        times[*index] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
            new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
            );
        return AsyncResult();
    };

    auto parallel5 = ParallelFor(manager,
        [](std::shared_ptr<void> data)->AsyncResult {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    return AsyncResult();
        }, 5).execute();

    auto parallel25 = ParallelFor(manager,
        [](std::shared_ptr<void> data)->AsyncResult {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            return AsyncResult();
        }, 25).execute();

    auto parallelTimes = ParallelFor(manager, func, nbTasks).execute(
        [&times](AsyncResult& res)->AsyncResult {
            if(!res.wasError())
            {
                times[5] = std::unique_ptr<std::chrono::high_resolution_clock::time_point>(
                    new std::chrono::high_resolution_clock::time_point(std::chrono::high_resolution_clock::now())
                    );
            }
            return std::move(res);
        } );


    std::function<AsyncResult()> ops[] = {
        [&parallel5]()->AsyncResult {
            return parallel5.get();
        },
        [&parallel25]()->AsyncResult {
            return parallel25.get();
        }, 
        []()->AsyncResult {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            return AsyncResult();
        },
        []()->AsyncResult {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            return AsyncResult();
        },
        [&parallelTimes]()->AsyncResult {
            return parallelTimes.get();
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