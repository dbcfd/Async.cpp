#include "async_cpp/async/AsyncResult.h"
#include "async_cpp/async/Filter.h"

#include "async_cpp/tasks/Manager.h"

#include <chrono>

#pragma warning(disable:4251)
#include <gtest/gtest.h>

using namespace async_cpp;
using namespace async_cpp::async;

TEST(FILTER_TEST, BASIC)
{
    auto manager(std::make_shared<tasks::Manager>(5));

    std::vector<std::shared_ptr<int>> data;
    data.emplace_back(std::make_shared<int>(1));
    data.emplace_back(std::make_shared<int>(2));
    data.emplace_back(std::make_shared<int>(3));
    data.emplace_back(std::make_shared<int>(3));
    data.emplace_back(std::make_shared<int>(4));
    data.emplace_back(std::make_shared<int>(5));
    data.emplace_back(std::make_shared<int>(6));
    data.emplace_back(std::make_shared<int>(2));
    data.emplace_back(std::make_shared<int>(7));
    data.emplace_back(std::make_shared<int>(4));

    auto op = [](std::shared_ptr<int> a) -> bool {
        return *a % 2 == 0;
    };

    auto finishOp = [](const AsyncResult<std::vector<std::shared_ptr<int>>>& result) -> std::future<AsyncResult<std::vector<std::shared_ptr<int>>>> {
        return result.asFulfilledFuture();
    };

    AsyncResult<std::vector<std::shared_ptr<int>>> result;
    result = Filter<int, std::vector<std::shared_ptr<int>>>(manager, op, data).execute(finishOp).get();
    ASSERT_NO_THROW(result.throwOrGet());
    std::shared_ptr<std::vector<std::shared_ptr<int>>> filterData;
    ASSERT_NO_THROW(filterData = result.throwOrGet());
    ASSERT_TRUE(filterData);
    ASSERT_EQ(5, filterData->size());
    for(auto i = 0; i < filterData->size(); ++i)
    {
        EXPECT_TRUE(*(filterData->at(i)) % 2 == 0);
    }

    manager->shutdown();
}