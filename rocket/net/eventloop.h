#pragma once
#include "rocket/common/god.h"
#include "rocket/net/fd_event.h"
#include "rocket/net/timer.h"
#include "timer_event.h"
#include "wakeup_fd_event.h"
#include <functional>

namespace rocket {

class EventLoop {
public:
    EventLoop();
    ~EventLoop();
    void loop();
    void stop();
    void addEpollEvent(FdEvent* event);
    void delEpollEvent(FdEvent* event);
    void wakeup();
    bool isInLoopThread();
    void addTask(std::function<void()>, bool is_wake_up = false);
    void addTimerEvent(TimerEvent::s_ptr);
private:
    void dealWakeUp();

    void initWakeUpFdEvent();

    void initTimer();

    WakeUpFdEvent* m_wakeup_fd_event = nullptr;

    pid_t m_pid;

    pid_t m_thread_id;

    int m_epoll_fd = -1;

    int m_wakeup_fd {-1};

    bool m_stop_flag = false;

    set<int> m_listen_fds;

    queue<function<void()>> m_pending_tasks;

    mutex m_mut;

    Timer* m_timer = nullptr;
};

}