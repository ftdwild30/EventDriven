//
// Created by ftd.wild on 2020/3/16.
//

#ifndef EVENTDRIVEN_TASK_QUEUE_SELECT_H
#define EVENTDRIVEN_TASK_QUEUE_SELECT_H

#include <cstddef>
#include <memory>
#include <list>
#include <mutex>
#include <thread>

namespace ftdwild {


class TaskEntry {
public:
    TaskEntry() = default;
    virtual ~TaskEntry() = default;

    virtual void Run() = 0;
};

typedef std::list<std::unique_ptr<TaskEntry>> TASK_LIST;
class TaskQueue {
public:
    TaskQueue();
    ~TaskQueue();

    int Start();

    void Stop();

    void AddTask(std::unique_ptr<TaskEntry> task);


private:
    static int makeSocketNonBlock(int fd);
    void threadEntry();
    int doTask();

private:
    int fd_in_;
    int fd_out_;
    bool running_;
    bool exit_;
    std::mutex mutex_;
    std::thread *thread_;
    TASK_LIST list_;

private:
    static const uint32_t kDefaultTaskNum = 256;
};


} //ftdwild



#endif //EVENTDRIVEN_TASK_QUEUE_SELECT_H
