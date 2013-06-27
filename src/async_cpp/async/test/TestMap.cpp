#include "async_cpp/async/AsyncResult.h"
#include "async_cpp/async/Map.h"

#include "async_cpp/tasks/Manager.h"

#include <chrono>

#pragma warning(disable:4251)
#include <gtest/gtest.h>

using namespace async_cpp;
using namespace async_cpp::async;

TEST(UNIQUE_TEST, BASIC)
{
    auto manager(std::make_shared<tasks::Manager>(5));

    std::vector<std::shared_ptr<int>> data;
    data.emplace_back(std::make_shared<int>(1));
    data.emplace_back(std::make_shared<int>(2));
    data.emplace_back(std::make_shared<int>(3));
    data.emplace_back(std::make_shared<int>(4));
    data.emplace_back(std::make_shared<int>(5));

    auto op = [](std::shared_ptr<int> a) -> std::shared_ptr<int> {
        return std::make_shared<int>(*a * *a);
    };

    auto finishOp = [](const AsyncResult<std::vector<std::shared_ptr<int>>>& result) -> std::future<AsyncResult<std::vector<std::shared_ptr<int>>>> {
        return result.asFulfilledFuture();
    };

    AsyncResult<std::vector<std::shared_ptr<int>>> result;
    result = Map<int, int, std::vector<std::shared_ptr<int>>>(manager, op, data).execute(finishOp).get();
        ASSERT_NO_THROW(result.throwOrGet());
    std::shared_ptr<std::vector<std::shared_ptr<int>>> mapData;
    ASSERT_NO_THROW(mapData = result.throwOrGet());
    ASSERT_TRUE(mapData);
    ASSERT_EQ(7, mapData->size());
    std::sort(mapData->begin(), mapData->end(), [](std::shared_ptr<int> a, std::shared_ptr<int> b) -> bool {
        return *a < *b;
    } );
    for(auto i = 0; i < mapData->size(); ++i)
    {
        EXPECT_EQ((i+1)*(i+1), *(mapData->at(i)));
    }

    manager->shutdown();
}