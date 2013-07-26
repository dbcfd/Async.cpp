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
        [](OpResult<size_t>&&, Series<size_t>::callback_t cb)-> void {
            cb(OpResult<size_t>(0));
        },
        [](OpResult<size_t>&& result, Series<size_t>::callback_t cb)-> void {
            cb(OpResult<size_t>(result.throwOrMove() + 1));
        },
        [](OpResult<size_t>&& result, Series<size_t>::callback_t cb)-> void {
            cb(OpResult<size_t>(result.throwOrMove() + 1));
        },
        [](OpResult<size_t>&& result, Series<size_t>::callback_t cb)-> void {
            cb(OpResult<size_t>(result.throwOrMove() + 1));
        },
        [](OpResult<size_t>&& result, Series<size_t>::callback_t cb)-> void {
            cb(OpResult<size_t>(result.throwOrMove() + 1));
        }
    };
    
    auto future = Series<size_t>(manager, opsArray, 5).then(
        [](OpResult<size_t>&& result, Series<size_t>::complete_t cb)-> void {
            if(result.wasError())
            {
                cb(AsyncResult(result.error()));
            }
            else
            {
                auto wasSuccessful = (4 == result.move());
                cb(AsyncResult());
            }
        } );
		
### Parallel ###
Run a set of tasks in parallel, where the final results are passed to the completion function

    Parallel<bool>::operation_t opsArray[] = {
        [&taskRunOrder, &runCount](Parallel<bool>::callback_t cb)->void { 
            taskRunOrder[0] = runCount.fetch_add(1); 
            cb(OpResult<bool>());
        }, 
        [&taskRunOrder, &runCount](Parallel<bool>::callback_t cb)->void { 
            taskRunOrder[1] = runCount.fetch_add(1); 
            cb(OpResult<bool>());
        },
        [&taskRunOrder, &runCount](Parallel<bool>::callback_t cb)->void { 
            taskRunOrder[2] = runCount.fetch_add(1); 
            cb(OpResult<bool>());
        },
        [&taskRunOrder, &runCount](Parallel<bool>::callback_t cb)->void { 
            taskRunOrder[3] = runCount.fetch_add(1); 
            cb(OpResult<bool>());
        },
        [&taskRunOrder, &runCount](Parallel<bool>::callback_t cb)->void { 
            taskRunOrder[4] = runCount.fetch_add(1); 
            cb(OpResult<bool>());
        }
    };
    
    auto future = Parallel<bool>(manager, opsArray, 5).then(
        [&taskRunOrder, &runCount](OpResult<Parallel<bool>::result_set_t>&& result, Parallel<bool>::complete_t cb)->void {
            if(result.wasError())
            {
                cb(AsyncResult(result.error()));
            }
            else
            {
                auto results = result.move();
                cb(AsyncResult());
            }
        } );
	
### Parallel For ###
Run an operation a set number of times in parallel, where the final results are passed to the completion function

    auto func = [&times](size_t index, ParallelFor<data_t>::callback_t cb)->void {
        auto now = std::chrono::high_resolution_clock::now();
        times[index] = std::make_shared<data_t>(now);
        cb(OpResult<data_t>(std::move(now)));
    };

    ParallelFor<data_t> parallel(manager, func, 5);
    auto maxDur = std::chrono::high_resolution_clock::duration::min();
    auto start = std::chrono::high_resolution_clock::now();
    auto future = parallel.then([&times, maxDur, this](OpResult<ParallelFor<data_t>::result_set_t>&& result, ParallelFor<data_t>::complete_t cb)->void {
        if(result.wasError())
        {
            cb(AsyncResult(result.error()));
        }
        else
        {
            auto results = result.move();
            cb(AsyncResult());
        }
    } );
	
### Parallel ForEach ###
Run an operation over a set of data, where the final results are passed to the completion function

    auto func = [&times](size_t& index, ParallelForEach<size_t, result_t>::callback_t cb)->void {
        auto now = std::chrono::high_resolution_clock::now();
        times[index] = std::make_shared<std::chrono::high_resolution_clock::time_point>(now);
        cb(OpResult<result_t>(std::move(now)));
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
    auto future = parallel.then([&times, &maxDur](OpResult<ParallelForEach<size_t, result_t>::result_set_t>&& result, ParallelForEach<size_t, result_t>::complete_t cb)->void {
        if(result.wasError())
        {
            cb(AsyncResult(result.error()));
        }
        else
        {
            auto results = result.move();
            cb(AsyncResult());
        }
    } );
    
### Map ###
Run an operation over a set of data that converts an item to another item, where the final results are passed to the completiion function.

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
            auto results = result.move();
            cb(AsyncResult());
        }
    };
    
### Filter ###
Filter a set of data based on some function, where the filtered results are passed to the completion function.

    auto op = [](const int& a) -> bool {
        return a % 2 == 0;
    };

    auto finishOp = [](OpResult<std::vector<int>>&& result, Filter<int>::callback_t cb) -> void {
        if(result.wasError())
        {
            cb(AsyncResult(result.error()));
        }
        else
        {
            auto results = result.move();
            cb(AsyncResult());
        }
    };
    
### Unique ###
Find a set of unique items in a set of data, based on some equality function. Unique items are passed to the completion function.

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
            auto results = result.move();
            cb(AsyncResult());
        }
    };

## Build Instructions ##
Obtain a C++11 compatible compiler (VS2011, Gcc) and CMake 2.8.4 or higher. Run Cmake (preferably from the build directory).

See http://www.cmake.org for further instructions on CMake.

## Testing Instructions ##
If flag BUILD_TESTS is enabled, google test based tests will be created for Tasks and Async. Alternative, RUN_TESTS project can be run.

## Known Issues ##

 * A large number of tasks which retain threads waiting for other threads to complete may cause a deadlock. 
  * Issue related to any thread pooling/event looping system (both Manager and AsioManager)
  * When possible, your tasks should not block, and instead return a std::future<AsyncResult<T>>. 
  * The design accommodates for this behavior
  * Manager and AsioManager have been tested against behavior similar to this, and it is not seen
