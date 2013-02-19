#include "workers/Manager.h"
#include "workers/Worker.h"
#include "workers/Task.h"

#pragma warning(disable:4251)
#include <gtest/gtest.h>

#include<thread>
using namespace quicktcp::workers;

class TestTask : public Task
{
public:
    TestTask() : wasPerformed(false)
    {

    }

    virtual ~TestTask()
    {

    }
    
    bool wasPerformed;

private:
    virtual void performSpecific()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        wasPerformed = true;
    }

};

TEST(WORKERS_TEST, TEST_TASK)
{
    {
        TestTask task;

        ASSERT_FALSE(task.wasPerformed);

        std::future<bool> future = task.getCompletionFuture();

        task.perform([](){});

        future.wait();

        ASSERT_TRUE(task.wasPerformed);
        ASSERT_TRUE(future.get());
    }

    {
        TestTask* task = new TestTask();

        ASSERT_FALSE(task->wasPerformed);

        std::future<bool> future = task->getCompletionFuture();

        delete task;

        future.wait();

        ASSERT_FALSE(future.get());
    }
}

class TestExceptionTask : public Task
{
public:
    TestExceptionTask()
    {

    }

    virtual ~TestExceptionTask()
    {

    }

private:
    virtual void performSpecific()
    {
        throw(std::runtime_error("Task threw an error"));
    }

};

TEST(WORKERS_TEST, TEST_EXCEPTION_TASK)
{
    {
        TestExceptionTask task;

        std::future<bool> future = task.getCompletionFuture();

        task.perform([](){});

        future.wait();

        ASSERT_FALSE(future.get()); //task failed due to exception
    }
}

class WorkerComplete
{
public:
    WorkerComplete() : hasCompleted(false)
    {

    }

    void complete()
    {
        hasCompleted = true;
    }

    bool hasCompleted;
};

TEST(WORKERS_TEST, TEST_WORKER)
{
    {
        //setup worker
        WorkerComplete workerComplete;
        auto completeFunction = [&workerComplete](Worker* worker) -> void { workerComplete.complete(); };
        Worker worker(completeFunction);
        worker.waitUntilReady();

        //setup and run task
        std::shared_ptr<Task> task1(new TestTask());
        worker.runTask(task1);
        std::future<bool> task1Future = task1->getCompletionFuture();
        task1Future.wait();

        //check that task was run
        ASSERT_TRUE(task1Future.get());
        ASSERT_TRUE(workerComplete.hasCompleted);

        //worker should shutdown and cleanup correctly
    }

    {
        //setup worker
        WorkerComplete workerComplete;
        auto completeFunction = [&workerComplete](Worker* worker) -> void { workerComplete.complete(); };
        Worker worker(completeFunction);
        worker.waitUntilReady();

        //shutdown worker at same time as running task
        std::shared_ptr<Task> task1(new TestTask());
        worker.runTask(task1);
        worker.shutdown();
        std::future<bool> task1Future = task1->getCompletionFuture();
        task1Future.wait();

        //task may or may not be run, dependend on when wakeup occurs compared to shutdown
        ASSERT_TRUE(workerComplete.hasCompleted);
    }

    {
        //setup worker
        WorkerComplete workerComplete;
        auto completeFunction = [&workerComplete](Worker* worker) -> void { workerComplete.complete(); };
        Worker worker(completeFunction);
        worker.waitUntilReady();

        //shutdown worker before running task
        std::shared_ptr<Task> task1(new TestTask());
        worker.shutdown();
        worker.runTask(task1);
        std::future<bool> task1Future = task1->getCompletionFuture();
        task1Future.wait();

        //we will fail to run task, which means worker will fail to look for another task
        ASSERT_FALSE(task1Future.get());
        ASSERT_FALSE(workerComplete.hasCompleted);
    }
}

TEST(WORKERS_TEST, MANAGER_TEST)
{
    {
        //setup a bunch of tasks
        std::vector< std::shared_ptr<Task> > tasks;
        for(size_t i = 0; i < 5; ++i) 
        {
            Task* task = new TestTask();
            tasks.push_back(std::shared_ptr<Task>(task));
        }

        //less workers than tasks, make sure they can go back and grab tasks
        Manager manager(2);

        for(std::vector< std::shared_ptr<Task> >::const_iterator task = tasks.begin(); task != tasks.end(); ++task)
        {
            manager.run((*task));
        }

        bool tasksCompleted = true;

        for(std::vector< std::shared_ptr<Task> >::const_iterator task = tasks.begin(); task != tasks.end(); ++task)
        {
            std::future<bool> taskFuture = (*task)->getCompletionFuture();

            taskFuture.wait();

            tasksCompleted &= taskFuture.get();
        }

        ASSERT_TRUE(tasksCompleted);

        //make sure cleanup shuts down correctly
    }

    {
        //setup a bunch of tasks
        std::vector< std::shared_ptr<Task> > tasks;
        for(size_t i = 0; i < 10; ++i) 
        {
            Task* task = new TestTask();
            tasks.push_back(std::shared_ptr<Task>(task));
        }

        //less workers than tasks, make sure they can go back and grab tasks
        Manager manager(2);

        bool tasksCompleted = true;

        //run some tasks
        manager.run(tasks[0]);
        manager.run(tasks[1]);
        manager.run(tasks[2]);
        manager.run(tasks[3]);

        {
            std::future<bool> taskFuture = tasks[0]->getCompletionFuture();
            taskFuture.wait();
            tasksCompleted &= taskFuture.get();
        }

        //run some more
        manager.run(tasks[4]);
        manager.run(tasks[5]);
        manager.run(tasks[6]);
        manager.run(tasks[7]);
        manager.run(tasks[8]);
        {
            std::future<bool> taskFuture = tasks[3]->getCompletionFuture();
            taskFuture.wait();
            tasksCompleted &= taskFuture.get();
        }

        manager.waitForTasksToComplete();

        int tasksToCheck[] = {1,2,4,5,6,7,8};

        for(size_t i = 0; i < 7; ++i)
        {
            std::future<bool> taskFuture = tasks[tasksToCheck[i]]->getCompletionFuture();
            taskFuture.wait();
            tasksCompleted &= taskFuture.get();
        }

        ASSERT_TRUE(tasksCompleted);

        manager.shutdown();

        manager.run(tasks[9]);

        {
            std::future<bool> taskFuture = tasks[9]->getCompletionFuture();
            taskFuture.wait();
            ASSERT_FALSE(taskFuture.get());
        }

        //make sure cleanup shuts down correctly
    }
}
