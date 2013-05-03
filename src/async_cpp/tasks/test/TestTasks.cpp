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

TEST(WORKERS_TEST, TEST_TASK)
{
    {
        TestTask task;

        ASSERT_FALSE(task.wasPerformed);

        task.perform([](){});

        ASSERT_TRUE(task.wasPerformed);
        ASSERT_TRUE(task.wasCompletedSuccessfully());
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

        task.perform([](){});

        ASSERT_FALSE(task.wasCompletedSuccessfully()); //task failed due to exception
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
        auto completeFunction = [&workerComplete](std::shared_ptr<Worker> worker) -> void { workerComplete.complete(); };
        auto worker = std::make_shared<Worker>(completeFunction);

        //setup and run task
        auto task1(std::make_shared<TestTask>());
        worker->runTask(task1);

        //check that task was run
        ASSERT_TRUE(task1->wasCompletedSuccessfully());

        //wait for worker to finish
        worker.reset();

        ASSERT_TRUE(workerComplete.hasCompleted);

        //worker should shutdown and cleanup correctly
    }

    {
        //setup worker
        WorkerComplete workerComplete;
        auto completeFunction = [&workerComplete](std::shared_ptr<Worker> worker) -> void { workerComplete.complete(); };
        auto worker = std::make_shared<Worker>(completeFunction);

        //shutdown worker at same time as running task
        auto task1 = std::make_shared<TestTask>();
        worker->runTask(task1);
        //just want to make sure that nothing bad happens when killing worker at approximately same time as running task
        ASSERT_NO_THROW(worker.reset());

        //shutdown after run, task may or may not complete
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

        for(auto task : tasks)
        {
            tasksCompleted &= task->wasCompletedSuccessfully();
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
            tasksCompleted &= tasks[0]->wasCompletedSuccessfully();
        }

        //run some more
        manager.run(tasks[4]);
        manager.run(tasks[5]);
        manager.run(tasks[6]);
        manager.run(tasks[7]);
        manager.run(tasks[8]);
        {
            tasksCompleted &= tasks[3]->wasCompletedSuccessfully();
        }

        manager.waitForTasksToComplete();

        int tasksToCheck[] = {1,2,4,5,6,7,8};

        for(size_t i = 0; i < 7; ++i)
        {
            tasksCompleted &= tasks[tasksToCheck[i]]->wasCompletedSuccessfully();
        }

        ASSERT_TRUE(tasksCompleted);

        manager.shutdown();

        manager.run(tasks[9]);

        {
            ASSERT_FALSE(tasks[9]->wasCompletedSuccessfully());
        }

        //make sure cleanup shuts down correctly
    }
}
