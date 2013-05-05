#include "async_cpp/tasks/Manager.h"
#include "async_cpp/tasks/Worker.h"
#include "async_cpp/tasks/Task.h"

#pragma warning(disable:4251)
#include <gtest/gtest.h>

#include<thread>
using namespace async_cpp::tasks;

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

TEST(TASKS_TEST, BASIC_TASK)
{
    TestTask task;

    EXPECT_FALSE(task.wasPerformed);

    task.perform();

    EXPECT_TRUE(task.wasPerformed);
    EXPECT_TRUE(task.wasSuccessful());
}

class RepeatedTask : public Task {
public:
    RepeatedTask() : wasPerformed(false), wasPerformedTwice(false)
    {

    }

    RepeatedTask(RepeatedTask&& other) : Task(), wasPerformed(other.wasPerformed), wasPerformedTwice(other.wasPerformedTwice)
    {

    }

    virtual ~RepeatedTask()
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
    }
};

TEST(TASKS_TEST, REPEATED_TASK)
{
    RepeatedTask task;

    EXPECT_FALSE(task.wasPerformed);
    EXPECT_FALSE(task.wasPerformedTwice);   

    task.perform();

    EXPECT_TRUE(task.wasPerformed);
    EXPECT_FALSE(task.wasPerformedTwice);
    EXPECT_TRUE(task.wasSuccessful());

    RepeatedTask task2(std::move(task));

    EXPECT_TRUE(task2.wasPerformed);
    EXPECT_FALSE(task2.isComplete());

    task2.perform();

    EXPECT_TRUE(task2.wasPerformed);
    EXPECT_TRUE(task2.wasPerformedTwice);
    EXPECT_TRUE(task2.wasSuccessful());
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

TEST(TASKS_TEST, EXCEPTION_TASK)
{
    TestExceptionTask task;

    task.perform();

    ASSERT_FALSE(task.wasSuccessful()); //task failed due to exception
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

TEST(TASKS_TEST, TEST_WORKER)
{
    {
        //setup worker
        WorkerComplete workerComplete;
        auto completeFunction = [&workerComplete](std::shared_ptr<Worker> worker) -> void { workerComplete.complete(); };
        auto worker = std::make_shared<Worker>(completeFunction, [](){});

        //setup and run task
        auto task1 = std::make_shared<TestTask>();
        worker->runTask(task1);

        //check that task was run
        ASSERT_TRUE(task1->wasSuccessful());

        //wait for worker to finish
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        ASSERT_TRUE(workerComplete.hasCompleted);

        //worker should shutdown and cleanup correctly
        worker.reset();
    }

    {
        //setup worker
        WorkerComplete workerComplete;
        auto completeFunction = [&workerComplete](std::shared_ptr<Worker> worker) -> void { workerComplete.complete(); };
        auto worker = std::make_shared<Worker>(completeFunction, [](){});

        //shutdown worker at same time as running task
        auto task1 = std::make_shared<TestTask>();
        worker->runTask(task1);
        //just want to make sure that nothing bad happens when killing worker at approximately same time as running task
        ASSERT_NO_THROW(worker.reset());

        //shutdown after run, task may or may not complete
    }
}

TEST(TASKS_TEST, MANAGER_TEST)
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

        for(auto task : tasks)
        {
            manager.run(task);
        }

        bool tasksCompleted = true;

        for(auto task : tasks)
        {
            tasksCompleted &= task->wasSuccessful();
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
