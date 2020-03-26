//
// Created by ftd.wild on 2020/3/16.
//

#include "task_queue_select.h"


#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

#include <chrono>


using namespace ftdwild;


TaskQueue::TaskQueue() {
    fd_in_ = 0;
    fd_out_ = 0;
    running_ = false;
    thread_ = nullptr;
    exit_ = false;
}

TaskQueue::~TaskQueue() {
    Stop();
}

void TaskQueue::AddTask(std::unique_ptr<TaskEntry> task) {
    std::lock_guard<std::mutex> lock(mutex_);
    list_.push_back(std::move(task));
    send(fd_in_, "t", 1, 0);
}

int TaskQueue::Start() {
    if (running_) {
        return 0;//只允许启动一次，简单处理
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
    exit_ = false;
    thread_ = new std::thread(&TaskQueue::threadEntry, this);
    thread_->detach();

    return 0;
}

void TaskQueue::Stop() {
    if (!running_) {
        return;
    }
    running_ = false;
    send(fd_in_, "e", 1, 0);

    while (!exit_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    delete thread_;
    if (fd_in_) {
        close(fd_in_);
        fd_in_ = 0;
    }
    if (fd_out_) {
        close(fd_out_);
        fd_out_ = 0;
    }
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

    exit_ = true;
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
            return 0;
        }
    }

    mutex_.lock();
    /* 把任务转移至新队列，减少锁的作用域 */
    TASK_LIST::iterator it = list_.begin();
    TASK_LIST new_list;
    i = 0;
    for (; it != list_.end() && i < ret; ) {
        new_list.push_back(std::move(*it));
        list_.erase(it++);
        i++;
    }
    mutex_.unlock();


    for (auto &it2 : new_list) {
        it2->Run();
    }

    return 0;
}
