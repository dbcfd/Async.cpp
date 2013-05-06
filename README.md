# Async.cpp #

## Overview ##
Asynchronous library modeled after async.js, using C++11. Define a set of tasks, then run those tasks in series or parallel, along with a task on completion.

Futures returned which indicate when the asynchronous operation is complete, and whether or not task was completed successfully.

### Tasks ###
Task based work system to simplify threading.

 * Task : Interface for any work which needs to be accomplished in a threaded manner. Can also be run non-threaded
 * Worker : Threaded object to handle performing tasks
 * IManager : Interface for managers which are responsible for running tasks
  * Manager : Collection of workers. Provides easy setup of a number of workers to run tasks
  * AsioManager : Uses boost::asio::io_service to run tasks

### Async ###
Asynchronous library modeled after async.js

 * AsyncResult: Result of an asynchronous task, may either contain data, or an error. 
  * Usage similar to javascript callback function(err, data). 
  * Implemented tasks should first check for error, forwarding AsyncResult if error is present. 
  * throwOrGet will throw a std::runtime_error if error, which can be used for forwarding
  * Placeholder for C++14 optional type
 * Series: Run a set of tasks in order, passing the result from one task to another. 
  *After all tasks are run, the optional completion task is called with the results from the series.
 * Parallel: Run a set of tasks in parallel. 
  *After all tasks are run, the optional completion task is called with a vector of results from the parallel tasks
 * ParallelFor: Run an operation for a set number of times, passing an index number to the operation. 
  *After all tasks are run, the optional completion task is called with a vector of results from the parallel tasks
 * ParallelForEach: Run an operation over a set of data in parallel. 
  *After all tasks are run, the optional completion task is called with a vector of results from the parallel tasks

## Examples ##

### Series ###
Run a set of tasks in series, where the final result is passed to the completion function.

    std::function<std::future<AsyncResult<size_t>>(const AsyncResult<size_t>&)> opsArray[] = {
        [](const AsyncResult<size_t>& in)->std::future<AsyncResult<size_t>> {
			return AsyncResult<size_t>(0).asFulfilledFuture();
        },
        [](const AsyncResult<size_t>& in)->std::future<AsyncResult<size_t>> {
    	    return AsyncResult<size_t>(*(in.throwOrGet()) + 1).asFulfilledFuture();
        },
        [](const AsyncResult<size_t>& in)->std::future<AsyncResult<size_t>> {
    	    return AsyncResult<size_t>(*(in.throwOrGet()) + 1).asFulfilledFuture();
        },
        [](const AsyncResult<size_t>& in)->std::future<AsyncResult<size_t>> {
    	    return AsyncResult<size_t>(*(in.throwOrGet()) + 1).asFulfilledFuture();
        },
        [](const AsyncResult<size_t>& in)->std::future<AsyncResult<size_t>> {
    	    return AsyncResult<size_t>(*(in.throwOrGet()) + 1).asFulfilledFuture();
        }
    };
    
    auto seriesFuture = async::Series<size_t, bool>(manager, opsArray, 5).execute(
        [](const AsyncResult<size_t>& in)-> std::future<AsyncResult<bool>> {
    	    bool wasSuccessful = (!in.wasError && (4 == (*in.data()) );
			return AsyncResult<bool>(wasSuccessful).asFulfilledFuture();
        } );
		
### Parallel ###
Run a set of tasks in parallel, where the final results are passed to the completion function

    auto func = []()->std::future<AsyncResult<data_t>> {
        return AsyncResult<data_t>(std::make_shared<data_t>(std::chrono::high_resolution_clock::now())).asFulfilledFuture();
    };

    auto ops = std::vector<std::function<std::future<AsyncResult<data_t>>(void)>>(5, func);

    Parallel<data_t, result_t> parallel(manager, ops);
    auto start = std::chrono::high_resolution_clock::now();
    auto future = parallel.execute([&start](const std::vector<AsyncResult<data_t>>& results)->std::future<AsyncResult<result_t>> {
        bool wasSuccessful = true;
        auto maxDur = std::chrono::high_resolution_clock::duration::min();
        for(auto& res : results)
        {
            try
            {
                auto taskExecute = res.throwOrGet();
                wasSuccessful = true;
                maxDur = std::max(maxDur, *taskExecute - start);
            }
            catch(std::runtime_error&)
            {
                wasSuccessful = false;
                break;
            }
        }
        return AsyncResult<result_t>(std::make_shared<result_t>(std::make_pair(maxDur, wasSuccessful))).asFulfilledFuture();
    } );
	
### Parallel For ###
Run an operation a set number of times in parallel, where the final results are passed to the completion function

    auto func = [&times](size_t index)->std::future<AsyncResult<size_t>> {
        times[index] = std::make_shared<std::chrono::high_resolution_clock::time_point>(std::chrono::high_resolution_clock::now());
        return AsyncResult<size_t>(std::make_shared<size_t>(index)).asFulfilledFuture();
    };

    ParallelFor<size_t, result_t> parallel(manager, func, 5);
    auto start = std::chrono::high_resolution_clock::now();
    auto future = parallel.execute([&times](const std::vector<AsyncResult<size_t>>& results)->std::future<AsyncResult<result_t>> {
        auto maxDur = std::chrono::high_resolution_clock::duration::min();
        for(auto& result : results)
        {
            auto dur = std::chrono::high_resolution_clock::now() - *(times[*(result.throwOrGet())]);
            maxDur = std::max(maxDur, dur);
        }
        return AsyncResult<result_t>(std::make_shared<result_t>(maxDur)).asFulfilledFuture();
    } );
	
### Parallel ForEach ###
Run an operation over a set of data, where the final results are passed to the completion function

    auto func = [&times](std::shared_ptr<size_t> index)->std::future<AsyncResult<size_t>> {
        times[*index] = std::make_shared<std::chrono::high_resolution_clock::time_point>(std::chrono::high_resolution_clock::now());
        return AsyncResult<size_t>(index).asFulfilledFuture();
    };

    std::vector<std::shared_ptr<size_t>> data;
    data.emplace_back(std::make_shared<size_t>(0));
    data.emplace_back(std::make_shared<size_t>(1));
    data.emplace_back(std::make_shared<size_t>(2));
    data.emplace_back(std::make_shared<size_t>(3));
    data.emplace_back(std::make_shared<size_t>(4));

    ParallelForEach<size_t, size_t, result_t> parallel(manager, func, data);
    auto start = std::chrono::high_resolution_clock::now();
    auto future = parallel.execute([&times](const std::vector<AsyncResult<size_t>>& results)->std::future<AsyncResult<result_t>> {
        auto maxDur = std::chrono::high_resolution_clock::duration::min();
        for(auto& result : results)
        {
            auto dur = std::chrono::high_resolution_clock::now() - *(times[*(result.throwOrGet())]);
            maxDur = std::max(maxDur, dur);
        }
        return AsyncResult<result_t>(std::make_shared<result_t>(maxDur)).asFulfilledFuture();
    } );

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
