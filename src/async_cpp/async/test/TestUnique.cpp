#include "async_cpp/async/AsyncResult.h"
#include "async_cpp/async/Unique.h"

#include "async_cpp/tasks/AsioManager.h"

#include <chrono>

#pragma warning(disable:4251)
#include <gtest/gtest.h>

using namespace async_cpp;
using namespace async_cpp::async;

TEST(UNIQUE_TEST, BASIC)
{
    auto manager(std::make_shared<tasks::AsioManager>(5));

    std::vector<int> data;
    data.emplace_back(1);
    data.emplace_back(2);
    data.emplace_back(3);
    data.emplace_back(3);
    data.emplace_back(4);
    data.emplace_back(5);
    data.emplace_back(6);
    data.emplace_back(2);
    data.emplace_back(7);
    data.emplace_back(4);

    auto equalOp = [](const int& a, const int& b) -> bool {
        return a == b;
    };

    auto finishOp = [](OpResult<std::vector<int>>&& result, Unique<int>::callback_t cb)->void {
        if(result.wasError())
        {
            cb(AsyncResult(result.error()));
        }
        else
        {
            bool isUnique = true;
            auto results = result.move();
            for(auto i = 0; i < results.size(); ++i)
            {
                auto val = i+1;
                isUnique = isUnique && (val == results[i]);
            }
            if(isUnique && results.size() == 7)
            {
                cb(AsyncResult());
            }
            else
            {
                cb(AsyncResult(std::string("not unique")));
            }
        }
    };

    AsyncResult result;
    result = Unique<int>(manager, equalOp, std::move(data)).then(finishOp).get();
    EXPECT_TRUE(result.wasSuccessful());

    manager->shutdown();
}