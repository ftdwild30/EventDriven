//
// Created by ftd.wild on 2020/3/16.
//

#include "task_queue_select.h"


#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>


using namespace ftdwild;


TaskQueue::TaskQueue() {
    fd_in_ = 0;
    fd_out_ = 0;
    running_ = false;
    start_ = false;
    thread_ = nullptr;
}

TaskQueue::~TaskQueue() {
    delete thread_;
}

void TaskQueue::AddTask(const std::shared_ptr<TaskEntry> &task) {
    std::lock_guard<std::mutex> lock(mutex_);
    list_.push_back(task);
    send(fd_in_, "t", 1, 0);
}

int TaskQueue::Start() {
    if (start_) {
        return -1;//只允许启动一次，简单处理
    }

    int sockets[2];
    int ret = socketpair(AF_LOCAL, SOCK_STREAM, 0, sockets);
    if (ret < 0) {
        printf("create socket failed, ret = %d\n", ret);
        return -1;
    }

    if (makeSocketNonBlock(sockets[0]) < 0 || makeSocketNonBlock(sockets[1]) < 0) {
        printf("make socket non-block failed\n");
        close(sockets[0]);
        close(sockets[1]);
        return -1;
    }
    fd_in_ = sockets[0];
    fd_out_ = sockets[1];

    running_ = true;
    thread_ = new std::thread(&TaskQueue::threadEntry, this);

    start_ = true;

    return 0;
}

void TaskQueue::Stop() {
    if (!start_) {
        return;
    }
    running_ = false;
    send(fd_in_, "e", 1, 0);
}

int TaskQueue::makeSocketNonBlock(int fd) {
    int flags;
    if ((flags = fcntl(fd, F_GETFL, nullptr)) < 0) {
        printf("fcntl failed\n");
        return -1;
    }

    if (!(flags &O_NONBLOCK)) {
        if (fcntl(fd, F_SETFL, flags & O_NONBLOCK) == -1) {
            printf("fcntl failed\n");
            return -1;
        }
    }

    return 0;
}

void TaskQueue::threadEntry() {
    fd_set read_set;
    while (running_) {
        FD_ZERO(&read_set);
        FD_SET(fd_out_, &read_set);
        int ret = select(fd_out_ + 1, &read_set, nullptr, nullptr, nullptr);
        if (ret < 0) {
            printf("select error\n");
            running_ = false;
            continue;
        }
        if (ret == 0) {
            continue;
        }

        if (FD_ISSET(fd_out_, &read_set)) {
            if (doTask() < 0) {
                printf("do task failed\n");
                running_ = false;
                continue;
            }
        }

    }
}

int TaskQueue::doTask() {
    char buf[kDefaultTaskNum];
    int ret = recv(fd_out_, buf, kDefaultTaskNum, 0);
    if (ret < 0) {
        printf("read task failed\n");
        return -1;
    }
    if (ret == 0) {
        return 0;
    }

    int i;
    for (i = 0; i < ret; i++) {
        if (buf[i] == 'e') {
            printf("recv exit cmd\n");
            return -1;
        }
    }

    mutex_.lock();
    /* 把任务转移至新队列，减少锁的作用域 */
    TASK_LIST::iterator it = list_.begin();
    TASK_LIST new_list;
    i = 0;
    for (; it != list_.end() && i < ret; ) {
        new_list.push_back(*it);
        list_.erase(it++);
        i++;
    }
    mutex_.unlock();

    for (auto &it2 : new_list) {
        it2->Run();
    }

    return 0;
}
