//
// Created by ftd.wild on 2020/3/26.
//

#ifndef EVENTDRIVEN_TEST_TASK_ENTRY_H
#define EVENTDRIVEN_TEST_TASK_ENTRY_H

#include "task_queue_select.h"


using namespace ftdwild;

class TestTaskEntry : public TaskEntry {
public:
    TestTaskEntry();
    virtual ~TestTaskEntry();

    virtual void Run();

private:
    static uint32_t id_;
};


#endif //EVENTDRIVEN_TEST_TASK_ENTRY_H
