# Async.cpp #

## Overview ##
Asynchronous library modeled after async.js, using C++11. Define a set of tasks, then run those tasks in series or parallel, along with a task on completion.

Futures returned which indicate when the asynchronous operation is complete.

### Workers ###
Task based work system to simplify threading.
* Task : Interface for any work which needs to be accomplished in a threaded manner. Can also be run non-threaded
* Worker : Threaded object to handle performing tasks
* Manager : Collection of workers. Provides easy setup of a number of workers

### Async ###
Asynchronous library modeled after async.js
* AsyncResult: Result of an asynchronous task, may either contain data, or an error. Usage similar to javascript callback function(err, data). Implemented tasks should first check for error, forwarding AsyncResult if error is present.
* Series: Run a set of tasks in order, passing the result from one task to another. After all tasks are run, the optional completion task is called with the results from the series.
* Parallel: Run a set of tasks in parallel. After all tasks are run, the optional completion task is called with a result indicating error or no error
* ParallelForEach: Run an operation over a set of data in parallel. After all tasks are run, a vector of results is passed to the optional completion task

## Examples ##
Run a set of tasks in series, where the final result is passed to the completion function.

    std::function<async::PtrAsyncResult(async::PtrAsyncResult)> opsArray[] = {
        [&runOrder, &runCount](async::PtrAsyncResult res)->async::PtrAsyncResult {
    		if(res.wasError()) return res;
            runOrder[0] = runCount.fetch_add(1);
            return res;
        },
        [&runOrder, &runCount](async::PtrAsyncResult res)->async::PtrAsyncResult {
    	    if(res.wasError()) return res;
            runOrder[1] = runCount.fetch_add(1);
            return res;
        },
        [&runOrder, &runCount](async::PtrAsyncResult res)->async::PtrAsyncResult {
    	    if(res.wasError()) return res;
            runOrder[2] = runCount.fetch_add(1);
            return res;
        },
        [&runOrder, &runCount](async::PtrAsyncResult res)->async::PtrAsyncResult {
    	    if(res.wasError()) return res;
            runOrder[3] = runCount.fetch_add(1);
            return res;
        },
        [&runOrder, &runCount](async::PtrAsyncResult res)->async::PtrAsyncResult {
    	    if(res.wasError()) return res;
            runOrder[4] = runCount.fetch_add(1);
            return PtrAsyncResult(new AsyncResult(std::shared_ptr<void>(new size_t(runCount))));
        }
    };
    
    std::future<async::PtrAsyncResult> seriesFuture = async::Series(manager, opsArray, 5).execute(
        [&runOrder](async::PtrAsyncResult result)->async::PtrAsyncResult {
    	    if(res.wasError()) return res;
            runOrder[5] = *(std::shared_pointer_cast<size_t>(result.result()));
            return result;
        } );

## Build Instructions ##
Obtain a C++11 compatible compiler (VS2011, Gcc) and CMake 2.8.4 or higher. Run Cmake (preferably from the build directory).

See http://www.cmake.org for further instructions on CMake.

## Testing Instructions ##
If flag BUILD_TESTS is enabled, google test based tests will be created for Workers and Async. Alternative, RUN_TESTS project can be run.
