# Async.cpp #

## Overview ##
Asynchronous library modeled after async.js, using C++11. Define a set of tasks, then run those tasks in series or parallel, along with a task on completion.

Futures returned which indicate when the asynchronous operation is complete, and whether or not task was completed successfully.

### Tasks ###
Task based work system to simplify threading.

 * Task : Interface for any work which needs to be accomplished in a threaded manner. Can also be run non-threaded
 * IManager : Interface for managers which are responsible for running tasks
  * AsioManager : Uses boost::asio::io_service to run tasks

### Async ###
Asynchronous library modeled after async.js

 * OpResult: Result of an asynchronous task, may either be successful with or without data, or contain an error 
  * Usage similar to javascript callback function(err, data). 
  * Implemented tasks should first check for error, forwarding error if present. 
  * throwOrMove will throw a std::runtime_error if error, which can be used for forwarding
  * Placeholder for C++14 optional type
 * Series: Run a set of tasks in order, passing the result from one task to another. 
  * After all tasks are run, the optional completion task is called with the results from the series.
 * Parallel: Run a set of tasks in parallel. 
  * After all tasks are run, the optional completion task is called with a vector of results from the parallel tasks
 * ParallelFor: Run an operation for a set number of times, passing an index number to the operation. 
  * After all tasks are run, the optional completion task is called with a vector of results from the parallel tasks
 * ParallelForEach: Run an operation over a set of data in parallel. 
  * After all tasks are run, the optional completion task is called with a vector of results from the parallel tasks
 * Filter: Filter a set of data based on a criteria
  * OpResult contains a filtered vector of data if no errors occur
 * Map: Map a set of data based on a function
  * OpResult contains a mapped vector of data if no errors occur
 * Unique: Filter a set of data based on equality comparison
  * OpResult contains a unique vector of data if no errors occur

## Examples ##

### Series ###
Run a set of tasks in series, where the final result is passed to the completion function.

    Series<size_t>::operation_t opsArray[] = {
        [](std::exception_ptr, size_t*, Series<size_t>::callback_t cb)-> void {
            cb(0);
        },
        [](std::exception_ptr ex, size_t* prev, Series<size_t>::callback_t cb)-> void {
            if(ex) std::rethrow_exception(ex);
            cb(*prev + 1);
        },
        [](std::exception_ptr ex, size_t* prev, Series<size_t>::callback_t cb)-> void {
            if(ex) std::rethrow_exception(ex);
            cb(*prev + 1);
        },
        [](std::exception_ptr ex, size_t* prev, Series<size_t>::callback_t cb)-> void {
            if(ex) std::rethrow_exception(ex);
            cb(*prev + 1);
        },
        [](std::exception_ptr ex, size_t* prev, Series<size_t>::callback_t cb)-> void {
            if(ex) std::rethrow_exception(ex);
            cb(*prev + 1);
        }
    };

    AsyncResult result;
    ASSERT_NO_THROW(result = Series<size_t>(manager, opsArray, 5).then(
        [](std::exception_ptr ex, size_t* prev)-> void 
        {
            if(ex) std::rethrow_exception(ex);
            
            auto wasSuccessful = (4 == *prev);
            if(!wasSuccessful) throw(std::runtime_error("Series failed"));
        } ) );

    EXPECT_NO_THROW(result.check());
		
### Parallel ###
Run a set of tasks in parallel, where the final results are passed to the completion function

    auto func = [](Parallel<data_t>::callback_t cb)->void { 
        cb(std::chrono::high_resolution_clock::now());
    };

    auto ops = std::vector<Parallel<data_t>::operation_t>(5, func);

    Parallel<data_t> parallel(manager, ops);
    AsyncResult result;
    auto start = std::chrono::high_resolution_clock::now();
    auto maxDur = std::chrono::high_resolution_clock::duration::min();
    result = parallel.then([&start, &maxDur](std::exception_ptr ex, std::vector<data_t>&& results)->void 
    {
        if(ex) std::rethrow_exception(ex);
        
        for(auto& taskFinish : results)
        {
            maxDur = std::max(maxDur, taskFinish - start);
        }
    } );
    ASSERT_NO_THROW(result.check());
	
### Parallel For ###
Run an operation a set number of times in parallel, where the final results are passed to the completion function

    auto func = [&times](size_t index, ParallelFor<data_t>::callback_t cb)->void {
        auto now = std::chrono::high_resolution_clock::now();
        times[index] = std::make_shared<data_t>(now);
        cb(std::move(now));
    };

    ParallelFor<data_t> parallel(manager, func, 5);
    auto maxDur = std::chrono::high_resolution_clock::duration::min();
    auto start = std::chrono::high_resolution_clock::now();
    auto result = parallel.then([&times, maxDur, this](std::exception_ptr ex, std::vector<data_t>&& results)->void 
    {
        if(ex) std::rethrow_exception(ex);

        for(size_t i = 0; i < results.size(); ++i)
        {
            auto& tp = results[i];
            auto prev = times[i];
            if(!prev)
            {
                throw(std::runtime_error("No previous time"));
            }
            if(*prev != tp)
            {
                throw(std::runtime_error("Time mismatch"));
            }
        }
    } );
	
### Parallel ForEach ###
Run an operation over a set of data, where the final results are passed to the completion function

    auto func = [&times](size_t& index, ParallelForEach<size_t, result_t>::callback_t cb)->void {
        auto now = std::chrono::high_resolution_clock::now();
        times[index] = std::make_shared<std::chrono::high_resolution_clock::time_point>(now);
        cb(std::move(now));
    };

    std::vector<size_t> data;
    data.emplace_back(0);
    data.emplace_back(1);
    data.emplace_back(2);
    data.emplace_back(3);
    data.emplace_back(4);

    ParallelForEach<size_t, result_t> parallel(manager, func, std::move(data));
    auto maxDur = std::chrono::high_resolution_clock::duration::min();
    auto start = std::chrono::high_resolution_clock::now();
    auto result = parallel.then([&times, &maxDur](std::exception_ptr ex, std::vector<result_t>&& results)->void 
    {
        if(ex) std::rethrow_exception(ex);

        for(auto& tp : results)
        {
            auto dur = std::chrono::high_resolution_clock::now() - tp;
            maxDur = std::max(maxDur, dur);
        }
    } );

    ASSERT_NO_THROW(result.check());
       
### Filter ###
Filter a set of data based on some function, where the filtered results are passed to the completion function.

    auto op = [](const int& a) -> bool {
        return a % 2 == 0;
    };

    auto finishOp = [](std::exception_ptr ex, std::vector<int>&& results) -> void 
    {
        if(ex) std::rethrow_exception(ex);

        bool filterCorrect = true;
        for(auto val : results)
        {
            filterCorrect = filterCorrect && ( val % 2 == 0);
        }
        if(!(filterCorrect && results.size() == 5))
        {
            throw(std::runtime_error("Filter incorrect")));
        }
    };

    auto result = Filter<int>(manager, op, std::move(data)).then(finishOp);
    EXPECT_NO_THROW(result.check());
    
### Map ###
Run an operation over a set of data that converts an item to another item, where the final results are passed to the completiion function.

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
    
### Unique ###
Find a set of unique items in a set of data, based on some equality function. Unique items are passed to the completion function.

    auto equalOp = [](const int& a, const int& b) -> bool {
        return a == b;
    };

    auto finishOp = [](std::exception_ptr ex, std::vector<int>&& results)->void {
        if(ex) std::rethrow_exception(ex);

        bool isUnique = true;
        for(auto i = 0; i < results.size(); ++i)
        {
            auto val = i+1;
            isUnique = isUnique && (val == results[i]);
        }
        if(!(isUnique && results.size() == 7))
        {
            throw(std::runtime_error("not unique")));
        }
    };

    auto result = Unique<int>(manager, equalOp, std::move(data)).then(finishOp);
    EXPECT_NO_THROW(result.check());

## Build Instructions ##
Obtain a C++11 compatible compiler (VS2011, Gcc) and CMake 2.8.4 or higher. Run Cmake (preferably from the build directory).

See http://www.cmake.org for further instructions on CMake.

## Testing Instructions ##
If flag BUILD_TESTS is enabled, google test based tests will be created for Tasks and Async. Alternative, RUN_TESTS project can be run.

## Gotchas ##
Each async function returns an AsyncResult. When combining multiple async functions (see TestOverload.cpp), you should not wait on the results of other async functions. AsyncResult's should always be moved into the callback, and async functions should never call check();

## Known Issues
 * A large number of tasks which retain threads waiting for other threads to complete may cause a deadlock. 
  * Issue related to any thread pooling/event looping system
  * When possible, your tasks should not block, and instead invoke the callback using an AsyncResult
