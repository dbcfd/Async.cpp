#ifdef HAS_BOOST
#include "async_cpp/tasks/AsioManager.h"
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

TEST(ASIO_MANAGER_TEST, BASIC_TEST)
{
    {
        //setup a bunch of tasks
        std::vector< std::shared_ptr<Task> > tasks;
        for(size_t i = 0; i < 5; ++i) 
        {
            tasks.emplace_back(std::make_shared<TestTask>());
        }

        //less workers than tasks, make sure they can go back and grab tasks
        AsioManager manager(2);

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
        AsioManager manager(2);

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

#endif
