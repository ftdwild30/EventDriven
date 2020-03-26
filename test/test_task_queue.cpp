//
// Created by ftd.wild on 2020/3/16.
//

#include <stdio.h>

#include <thread>
#include <chrono>

#include "test_task_entry.h"
#include "task_queue_select.h"

using namespace ftdwild;

int main() {
    printf("start\n");

    TaskQueue task_queue;
    if (task_queue.Start() < 0) {
        printf("start task queue failed");
        return -1;
    }

    for (int i = 0; i < 10; i++) {
        std::unique_ptr<TaskEntry> test_task_entry(new TestTaskEntry());
        task_queue.AddTask(std::move(test_task_entry));
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    task_queue.Stop();

    printf("exit\n");
    return 0;
}