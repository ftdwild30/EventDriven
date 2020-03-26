//
// Created by ftd.wild on 2020/3/26.
//

#include "test_task_entry.h"

uint32_t TestTaskEntry::id_ = 0;
TestTaskEntry::TestTaskEntry() : TaskEntry() {

}

TestTaskEntry::~TestTaskEntry() {

}

void TestTaskEntry::Run() {
    printf("Run time: %u \n", ++id_);
}
