//
// Created by m1393 on 2025/1/14.
//

#include "Timer.h"

// 启动定时器
void Timer::start(int milliseconds, std::function<void()> func) {
    stop(); // 确保之前的定时器已停止
    running = true;

    // 在新线程中运行定时器
    timerThread = std::thread([this, milliseconds, func]() {
        std::this_thread::sleep_for(std::chrono::milliseconds (milliseconds));
        if (running) {
            func(); // 调用传入的函数
        }
    });
}

// 停止定时器
void Timer::stop() {
    running = false;
    if (timerThread.joinable()) {
        timerThread.join(); // 等待线程结束
    }
}