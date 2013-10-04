#include "async_cpp/async/AsyncResult.h"
#include "async_cpp/async/Parallel.h"

#include "async_cpp/tasks/AsioManager.h"

#include <chrono>

#pragma warning(disable:4251)
#include <gtest/gtest.h>

using namespace async_cpp;
using namespace async_cpp::async;

class ParallelTest : public testing::Test {
public:
    virtual void SetUp() final
    {
        manager = std::make_shared<tasks::AsioManager>(5);
    }

    virtual void TearDown() final
    {
        manager->shutdown();
    }

    tasks::ManagerPtr manager;
};

TEST_F(ParallelTest, BASIC)
{
    std::atomic<int> runCount(1);
    std::vector<int> taskRunOrder(6, 0);

    Parallel<bool>::operation_t opsArray[] = {
        [&taskRunOrder, &runCount](Parallel<bool>::callback_t cb)->void { 
            taskRunOrder[0] = runCount.fetch_add(1); 
            cb(AsyncResult());
        }, 
        [&taskRunOrder, &runCount](Parallel<bool>::callback_t cb)->void { 
            taskRunOrder[1] = runCount.fetch_add(1); 
            cb(AsyncResult());
        },
        [&taskRunOrder, &runCount](Parallel<bool>::callback_t cb)->void { 
            taskRunOrder[2] = runCount.fetch_add(1); 
            cb(AsyncResult());
        },
        [&taskRunOrder, &runCount](Parallel<bool>::callback_t cb)->void { 
            taskRunOrder[3] = runCount.fetch_add(1); 
            cb(AsyncResult());
        },
        [&taskRunOrder, &runCount](Parallel<bool>::callback_t cb)->void { 
            taskRunOrder[4] = runCount.fetch_add(1); 
            cb(AsyncResult());
        }
    };

    AsyncResult result;
    ASSERT_NO_THROW(result = Parallel<bool>(manager, opsArray, 5).then(
        [&taskRunOrder, &runCount](std::exception_ptr ex, std::vector<bool>&&)->void {
            if(ex)
            {
                std::rethrow_exception(ex);
            }
            taskRunOrder[5] = runCount;
        } ) );

    EXPECT_NO_THROW(result.check());

    EXPECT_LE(5, runCount);
}

TEST_F(ParallelTest, DOUBLE_CALLBACK)
{
    std::atomic<int> runCount(1);
    std::vector<int> taskRunOrder(6, 0);

    Parallel<bool>::operation_t opsArray[] = {
        [&taskRunOrder, &runCount](Parallel<bool>::callback_t cb)->void { 
            taskRunOrder[0] = runCount.fetch_add(1); 
            cb(AsyncResult());
            cb(AsyncResult());
        }, 
        [&taskRunOrder, &runCount](Parallel<bool>::callback_t cb)->void { 
            taskRunOrder[1] = runCount.fetch_add(1); 
            cb(AsyncResult());
        },
        [&taskRunOrder, &runCount](Parallel<bool>::callback_t cb)->void { 
            taskRunOrder[2] = runCount.fetch_add(1); 
            cb(AsyncResult());
            cb(AsyncResult());
        },
        [&taskRunOrder, &runCount](Parallel<bool>::callback_t cb)->void { 
            taskRunOrder[3] = runCount.fetch_add(1); 
            cb(AsyncResult());
        },
        [&taskRunOrder, &runCount](Parallel<bool>::callback_t cb)->void { 
            taskRunOrder[4] = runCount.fetch_add(1); 
            cb(AsyncResult());
            cb(AsyncResult());
        }
    };

    AsyncResult result;
    ASSERT_NO_THROW(result = Parallel<bool>(manager, opsArray, 5).then(
        [&taskRunOrder, &runCount](std::exception_ptr ex, std::vector<bool>&&)->void {
            if(ex)
            {
                std::rethrow_exception(ex);
            }
            taskRunOrder[5] = runCount;
        } ) );

    EXPECT_NO_THROW(result.check());

    EXPECT_LE(5, runCount);
}

TEST_F(ParallelTest, INTERRUPT)
{
    std::atomic<int> runCount(1);
    std::vector<int> taskRunOrder(6, 0);

    Parallel<bool>::operation_t opsArray[] = {
        [&taskRunOrder, &runCount](Parallel<bool>::callback_t cb)->void { 
            taskRunOrder[0] = runCount.fetch_add(1); 
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            cb(AsyncResult());
        }, 
        [&taskRunOrder, &runCount](Parallel<bool>::callback_t cb)->void { 
            taskRunOrder[1] = runCount.fetch_add(1); 
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            cb(AsyncResult());
        },
        [&taskRunOrder, &runCount](Parallel<bool>::callback_t cb)->void { 
            taskRunOrder[2] = runCount.fetch_add(1); 
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            cb(AsyncResult());
        },
        [&taskRunOrder, &runCount](Parallel<bool>::callback_t cb)->void { 
            taskRunOrder[3] = runCount.fetch_add(1); 
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            cb(AsyncResult());
        },
        [&taskRunOrder, &runCount](Parallel<bool>::callback_t cb)->void { 
            taskRunOrder[4] = runCount.fetch_add(1); 
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            cb(AsyncResult());
        }
    };

    auto thenFunc = [&taskRunOrder, &runCount](std::exception_ptr ex, std::vector<bool>&&)->void
    {
        if(ex) std::rethrow_exception(ex);
        taskRunOrder[5] = runCount;
    };

    auto parallel = Parallel<bool>(manager, opsArray, 5);

    AsyncResult result;
    ASSERT_NO_THROW(result = parallel.then(thenFunc));

    parallel.cancel();

    EXPECT_THROW(result.check(), std::runtime_error);
}

TEST_F(ParallelTest, TIMING)
{
    typedef std::chrono::high_resolution_clock::time_point data_t;

    auto func = [](Parallel<data_t>::callback_t cb)->void { 
        cb(std::chrono::high_resolution_clock::now());
    };

    auto ops = std::vector<Parallel<data_t>::operation_t>(5, func);

    Parallel<data_t> parallel(manager, ops);
    AsyncResult result;
    auto start = std::chrono::high_resolution_clock::now();
    auto maxDur = std::chrono::high_resolution_clock::duration::min();
    result = parallel.then([&start, &maxDur](std::exception_ptr ex, std::vector<data_t>&& results)->void {
        if(ex)
        {
            std::rethrow_exception(ex);
        }
        for(auto& taskFinish : results)
        {
            maxDur = std::max(maxDur, taskFinish - start);
        }
    } );
    ASSERT_NO_THROW(result.check());
    auto totalDur = std::chrono::high_resolution_clock::now() - start;

    EXPECT_GE(totalDur, maxDur);
}

TEST_F(ParallelTest, PASS_ASYNC)
{
    std::shared_ptr<std::chrono::high_resolution_clock::time_point> p1Op, p1Then, p2Op, p2Then, join;

    auto op1 = [&p1Op, &p1Then, this](Parallel<bool>::callback_t cb)->void
        {
            Parallel<bool>::operation_t ops[] = {
                [&p1Op](Parallel<bool>::callback_t innerCb)->void
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    p1Op = std::make_shared<std::chrono::high_resolution_clock::time_point>(std::chrono::high_resolution_clock::now());
                    innerCb(true);
                }
            };

            cb(Parallel<bool>(manager, ops, 1).then(
                [&p1Then](std::exception_ptr ex, std::vector<bool>&&)->void
            {
                if(ex) std::rethrow_exception(ex);
                p1Then = std::make_shared<std::chrono::high_resolution_clock::time_point>(std::chrono::high_resolution_clock::now());
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            } ) );
        };

    auto op2 = [&p2Op, &p2Then, this](Parallel<bool>::callback_t cb)->void
        {
            Parallel<bool>::operation_t ops[] = {
                [&p2Op](Parallel<bool>::callback_t innerCb)->void
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    p2Op = std::make_shared<std::chrono::high_resolution_clock::time_point>(std::chrono::high_resolution_clock::now());
                    innerCb(true);
                }
            };

            cb(Parallel<bool>(manager, ops, 1).then(
                [&p2Then](std::exception_ptr ex, std::vector<bool>&&)->void
            {
                if(ex) std::rethrow_exception(ex);
                p2Then = std::make_shared<std::chrono::high_resolution_clock::time_point>(std::chrono::high_resolution_clock::now());
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            } ) );
        };

    Parallel<bool>::operation_t joinOps[] = { op1, op2 };

    auto thenFunc = [&join](std::exception_ptr ex, std::vector<bool>&&)->void 
    { 
        if(ex) std::rethrow_exception(ex);
        join = std::make_shared<std::chrono::high_resolution_clock::time_point>(std::chrono::high_resolution_clock::now()); 
    };

    ASSERT_NO_THROW(Parallel<bool>(manager, joinOps, 2).then(thenFunc).check());

    EXPECT_LE(p1Op->time_since_epoch().count(), p1Then->time_since_epoch().count());
    EXPECT_LE(p2Op->time_since_epoch().count(), p2Then->time_since_epoch().count());
    EXPECT_LT(p1Then->time_since_epoch().count(), join->time_since_epoch().count());
    EXPECT_LT(p2Then->time_since_epoch().count(), join->time_since_epoch().count());
}

TEST_F(ParallelTest, INNER_CALLBACK)
{
    std::chrono::high_resolution_clock::time_point p1Op, p1Then, p2Op, p2Then, join;

    auto op1 = [&p1Op, &p1Then, this](Parallel<bool>::callback_t cb)->void
        {
            Parallel<bool>::operation_t ops[] = {
                [&p1Op](Parallel<bool>::callback_t cb)->void
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    p1Op = std::chrono::high_resolution_clock::now();
                    cb(true);
                }
            };

            Parallel<bool>(manager, ops, 1).then(
                [&p1Then, cb](std::exception_ptr ex, std::vector<bool>&&)->void
            {
                if(ex) std::rethrow_exception(ex);
                p1Then = std::chrono::high_resolution_clock::now();
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                cb(true);
            } );
        };

    auto op2 = [&p2Op, &p2Then, this](Parallel<bool>::callback_t cb)->void
        {
            Parallel<bool>::operation_t ops[] = {
                [&p2Op](Parallel<bool>::callback_t cb)->void
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    p2Op = std::chrono::high_resolution_clock::now();
                    cb(true);
                }
            };

            Parallel<bool>(manager, ops, 1).then(
                [&p2Then, cb](std::exception_ptr ex, std::vector<bool>&&)->void
            {
                if(ex) std::rethrow_exception(ex);
                p2Then = std::chrono::high_resolution_clock::now();
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                cb(true);
            } );
        };

    Parallel<bool>::operation_t joinOps[] = { op1, op2 };

    auto thenFunc = [&join](std::exception_ptr ex, std::vector<bool>&&)->void 
    { 
        if(ex) std::rethrow_exception(ex);
        join = std::chrono::high_resolution_clock::now(); 
    };

    ASSERT_NO_THROW(Parallel<bool>(manager, joinOps, 2).then(thenFunc).check());

    EXPECT_LE(p1Op.time_since_epoch().count(), p1Then.time_since_epoch().count());
    EXPECT_LE(p2Op.time_since_epoch().count(), p2Then.time_since_epoch().count());
    EXPECT_LT(p1Then.time_since_epoch().count(), join.time_since_epoch().count());
    EXPECT_LT(p2Then.time_since_epoch().count(), join.time_since_epoch().count());
}