#include "async_cpp/async/AsyncResult.h"
#include "async_cpp/async/Unique.h"

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
    data.emplace_back(std::make_shared<int>(3));
    data.emplace_back(std::make_shared<int>(4));
    data.emplace_back(std::make_shared<int>(5));
    data.emplace_back(std::make_shared<int>(6));
    data.emplace_back(std::make_shared<int>(2));
    data.emplace_back(std::make_shared<int>(7));
    data.emplace_back(std::make_shared<int>(4));

    auto op = [](std::shared_ptr<int> a, std::shared_ptr<int> b) -> bool {
        return *a == *b;
    };

    auto finishOp = [](const AsyncResult<std::vector<std::shared_ptr<int>>>& result) -> std::future<AsyncResult<std::vector<std::shared_ptr<int>>>> {
        return result.asFulfilledFuture();
    };

    AsyncResult<std::vector<std::shared_ptr<int>>> result;
    result = Unique<int, std::vector<std::shared_ptr<int>>>(manager, op, data).execute(finishOp).get();
        ASSERT_NO_THROW(result.throwOrGet());
    std::shared_ptr<std::vector<std::shared_ptr<int>>> uniqueData;
    ASSERT_NO_THROW(uniqueData = result.throwOrGet());
    ASSERT_TRUE(uniqueData);
    ASSERT_EQ(7, uniqueData->size());
    std::sort(uniqueData->begin(), uniqueData->end(), [](std::shared_ptr<int> a, std::shared_ptr<int> b) -> bool {
        return *a < *b;
    } );
    for(auto i = 0; i < uniqueData->size(); ++i)
    {
        EXPECT_EQ(i+1, *(uniqueData->at(i)));
    }

    manager->shutdown();
}