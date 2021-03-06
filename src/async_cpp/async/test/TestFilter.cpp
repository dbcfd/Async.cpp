#include "async_cpp/async/AsyncResult.h"
#include "async_cpp/async/Filter.h"

#include "async_cpp/tasks/AsioManager.h"

#include <chrono>

#pragma warning(disable:4251)
#include <gtest/gtest.h>

using namespace async_cpp;
using namespace async_cpp::async;

TEST(FILTER_TEST, BASIC)
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

    auto op = [](const int& a) -> bool {
        return a % 2 == 0;
    };

    auto finishOp = [](std::exception_ptr ex, std::vector<int>&& results) -> void {
        if(ex) std::rethrow_exception(ex);

        bool filterCorrect = true;
        for(auto val : results)
        {
            filterCorrect = filterCorrect && ( val % 2 == 0);
        }
        if(!(filterCorrect && results.size() == 5))
        {
            throw(std::runtime_error("Filter incorrect"));
        }
    };

    auto result = Filter<int>(manager, op, std::move(data)).then(finishOp);
    EXPECT_NO_THROW(result.check());
    
    manager->shutdown();
}