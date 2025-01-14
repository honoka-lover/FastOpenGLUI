//
// Created by m1393 on 2025/1/14.
//

#ifndef FASTOPENGLUI_TIMER_H
#define FASTOPENGLUI_TIMER_H


#include <iostream>
#include <thread>
#include <functional>
#include <chrono>
#include <atomic>

class Timer {
public:
    Timer() : running(false) {}

    ~Timer() {
        stop();
    }

    // 启动定时器
    void start(int milliseconds, std::function<void()> func);

    // 停止定时器
    void stop();

private:
    std::thread timerThread;        // 定时器线程
    std::atomic<bool> running;      // 标志定时器是否运行
};



#endif //FASTOPENGLUI_TIMER_H
