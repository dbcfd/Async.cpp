#include "async_cpp/tasks/Task.h"

#pragma warning(disable:4251)
#include <gtest/gtest.h>

#include<thread>
using namespace async_cpp::tasks;

class TestTask : public Task
{
public:
    TestTask() : wasPerformed(false), failedToPerform(false)
    {

    }

    virtual ~TestTask()
    {

    }
    
    bool wasPerformed;
    bool failedToPerform;

private:
    virtual void performSpecific() final
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        wasPerformed = true;
    }

    virtual void notifyFailureToPerform() final
    {
        failedToPerform = true;
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
    virtual void performSpecific() final
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
    TestExceptionTask() : hadException(false)
    {

    }

    virtual ~TestExceptionTask()
    {

    }

    bool hadException;

private:
    virtual void performSpecific()
    {
        throw(std::runtime_error("Task threw an error"));
    }

    virtual void notifyException(const std::exception&) final
    {
        hadException = true;
    }
    

};

TEST(TASKS_TEST, EXCEPTION_TASK)
{
    TestExceptionTask task;

    task.perform();

    EXPECT_FALSE(task.wasSuccessful()); //task failed due to exception
    EXPECT_TRUE(task.hadException);
}