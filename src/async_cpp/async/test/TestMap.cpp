#include "async_cpp/async/AsyncResult.h"
#include "async_cpp/async/Map.h"

#include "async_cpp/tasks/AsioManager.h"

#include <chrono>

#pragma warning(disable:4251)
#include <gtest/gtest.h>

using namespace async_cpp;
using namespace async_cpp::async;

TEST(MAP_TEST, BASIC)
{
    typedef std::pair<int, int> result_t;
    auto manager(std::make_shared<tasks::AsioManager>(5));

    std::vector<int> data;
    data.emplace_back(1);
    data.emplace_back(2);
    data.emplace_back(3);
    data.emplace_back(4);
    data.emplace_back(5);
    
    auto mapOp = [](int& a) -> result_t {
        return std::make_pair(a, a*a);
    };

    auto finishOp = [](OpResult<std::vector<result_t>>&& result, Map<int, result_t>::callback_t cb)->void
    {
        if(result.wasError())
        {
            cb(AsyncResult(result.error()));
        }
        else
        {
            bool successful = true;
            auto results = result.move();
            for(size_t i = 0; i < results.size(); ++i)
            {
                auto val = i+1;
                successful = successful && ( val == results[i].first && val*val == results[i].second);
            }
            if(successful)
            {
                cb(AsyncResult());
            }
            else
            {
                cb(AsyncResult(std::string("Match failure")));
            }
        }
    };

    AsyncResult result;
    result = Map<int, result_t>(manager, mapOp, std::move(data)).then(finishOp).get();
    EXPECT_TRUE(result.wasSuccessful());

    manager->shutdown();
}