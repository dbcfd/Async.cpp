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
    
    auto mapOp = [](const int& a) -> result_t {
        return std::make_pair(a, a*a);
    };

    auto finishOp = [](std::exception_ptr ex, std::vector<result_t>&& results)->void
    {
        if(ex) std::rethrow_exception(ex);
        
        bool successful = true;
        for(size_t i = 0; i < results.size(); ++i)
        {
            auto val = i+1;
            successful = successful && ( val == results[i].first && val*val == results[i].second);
        }
        if(!successful)
        {
            throw(std::runtime_error("Match failure"));
        }
    };

    auto result = Map<int, result_t>(manager, mapOp, std::move(data)).then(finishOp);
    EXPECT_NO_THROW(result.check());

    manager->shutdown();
}