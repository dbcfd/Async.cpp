#include "async_cpp/tasks/Manager.h"
#include "async_cpp/tasks/Worker.h"
#include "async_cpp/tasks/Task.h"

#pragma warning(disable:4251)
#include <gtest/gtest.h>

#include<thread>
using namespace async_cpp::workers;

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
        ASSERT_FALSE(task.isComplete());

        task.perform([](){});

        ASSERT_TRUE(task.wasPerformed);
        ASSERT_TRUE(task.isComplete());
        ASSERT_TRUE(task.wasSuccessful()); //complete, so won't block
    }
}

class TestRepeatedTask : public Task
{
public:
    TestRepeatedTask()
    {
        wasPerformed = false;
        wasPerformedTwice = false;
    }

    virtual ~TestRepeatedTask()
    {

    }
    
    bool wasPerformed;
    bool wasPerformedTwice;

private:
    virtual void performSpecific()
    {
        if(wasPerformed)
        {
            wasPerformedTwice = true;
        }
        wasPerformed = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
};

TEST(WORKERS_TEST, TEST_REPEATED_TASK)
{
    TestRepeatedTask task;

    ASSERT_FALSE(task.wasPerformed);
    ASSERT_FALSE(task.isComplete());

    task.perform([](){});

    ASSERT_TRUE(task.wasPerformed);
    ASSERT_TRUE(task.isComplete());
    ASSERT_FALSE(task.wasPerformedTwice);
    ASSERT_TRUE(task.wasSuccessful());

    task.reset();

    ASSERT_FALSE(task.isComplete());

    task.perform([](){});

    ASSERT_TRUE(task.wasPerformed);
    ASSERT_TRUE(task.isComplete());
    ASSERT_TRUE(task.wasPerformedTwice);
    ASSERT_TRUE(task.wasSuccessful());
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

        task.perform([](){});

        ASSERT_TRUE(task.isComplete());
        ASSERT_FALSE(task.wasSuccessful()); //task failed due to exception
    }
}

class WorkerComplete
{
public:
    WorkerComplete() : hasCompleted(false), wasRunning(true)
    {

    }

    void complete(const bool running)
    {
        hasCompleted = true;
        wasRunning = running;
    }

    bool hasCompleted;
    bool wasRunning;
};

TEST(WORKERS_TEST, TEST_WORKER)
{
    {
        //setup worker
        WorkerComplete workerComplete;
        auto completeFunction = [&workerComplete](Worker* worker) -> void { workerComplete.complete(worker->isRunning()); };
        Worker worker(completeFunction);

        //setup and run task
        auto task1(std::make_shared<TestTask>());
        worker.runTask(task1);

        //check that task was run
        ASSERT_TRUE(task1->wasSuccessful());

        //wait for worker to finish
        worker.shutdown();

        ASSERT_TRUE(workerComplete.hasCompleted);

        //worker should shutdown and cleanup correctly
    }

    {
        //setup worker
        WorkerComplete workerComplete;
        auto completeFunction = [&workerComplete](Worker* worker) -> void { workerComplete.complete(worker->isRunning()); };
        Worker worker(completeFunction);

        //shutdown worker at same time as running task
        auto task1(std::make_shared<TestTask>());
        worker.runTask(task1);
        worker.shutdown();

        //shutdown after run, task may not get run
        ASSERT_TRUE(workerComplete.hasCompleted);
    }

    {
        //setup worker
        WorkerComplete workerComplete;
        auto completeFunction = [&workerComplete](Worker* worker) -> void { workerComplete.complete(worker->isRunning()); };
        Worker worker(completeFunction);

        //shutdown worker before running task
        auto task1(std::make_shared<TestTask>());
        worker.shutdown();
        worker.runTask(task1);

        //we will fail to run task, which means worker will fail to look for another task
        ASSERT_FALSE(task1->wasSuccessful());
        //worker was shutdown before run, complete will be marked as false
        ASSERT_TRUE(workerComplete.hasCompleted);
        ASSERT_FALSE(workerComplete.wasRunning);
    }
}

TEST(WORKERS_TEST, MANAGER_TEST)
{
    {
        //setup a bunch of tasks
        std::vector< std::shared_ptr<Task> > tasks;
        for(size_t i = 0; i < 5; ++i) 
        {
            tasks.emplace_back(std::make_shared<TestTask>());
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
            tasksCompleted &= (*task)->wasSuccessful();
        }

        ASSERT_TRUE(tasksCompleted);

        //make sure cleanup shuts down correctly
    }

    {
        //setup a bunch of tasks
        std::vector< std::shared_ptr<Task> > tasks;
        for(size_t i = 0; i < 10; ++i) 
        {
            tasks.emplace_back(std::make_shared<TestTask>());
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
            tasksCompleted &= tasks[0]->wasSuccessful();
        }

        //run some more
        manager.run(tasks[4]);
        manager.run(tasks[5]);
        manager.run(tasks[6]);
        manager.run(tasks[7]);
        manager.run(tasks[8]);
        {
            tasksCompleted &= tasks[3]->wasSuccessful();
        }

        manager.waitForTasksToComplete();

        int tasksToCheck[] = {1,2,4,5,6,7,8};

        for(size_t i = 0; i < 7; ++i)
        {
            tasksCompleted &= tasks[tasksToCheck[i]]->wasSuccessful();
        }

        ASSERT_TRUE(tasksCompleted);

        manager.shutdown();

        manager.run(tasks[9]);

        {
            ASSERT_FALSE(tasks[9]->wasSuccessful());
        }

        //make sure cleanup shuts down correctly
    }
}
