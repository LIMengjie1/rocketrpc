#include <cstdint>
#include <unistd.h>
#include <mutex>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include "rocket/net/eventloop.h"
#include "fd_event.h"
#include "rocket/common/log.h"
#include "util.h"
#include "wakeup_fd_event.h"

static thread_local int g_no = 0;
#define  ADD_TO_EPOLL() \
    auto it = m_listen_fds.find(event->getFd()); \
    int op = EPOLL_CTL_ADD; \
    if (it != m_listen_fds.end()) { \
        op = EPOLL_CTL_MOD; \
    } \
    epoll_event tmp = event->getEpollEvent(); \
    int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp); \
    if (rt == -1) { \
        ERRORLOG("failed epoll_ctl when add fd, errno=%d, error=%s", errno, strerror(errno)); \
    } \
    m_listen_fds.insert(event->getFd()); \
    g_no = tmp.events; \
    DEBUGLOG("add event succ %d, value:%u", event->getFd(), g_no); \

#define DEL_TO_EPOLL() \
    auto it = m_listen_fds.find(event->getFd()); \
    int op = EPOLL_CTL_DEL; \
    if (it == m_listen_fds.end()) { \
        return; \
    } \
    epoll_event tmp = event->getEpollEvent(); \
    int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp); \
    if (rt == -1) { \
        ERRORLOG("failed to del epoll event"); \
    } \
    m_listen_fds.erase(event->getFd()); \
    g_no =tmp.events; \
    DEBUGLOG("del event succ %d, value:%u", event->getFd(), g_no); \


namespace rocket {

static thread_local EventLoop* t_current_eventloop = nullptr;
static int g_epoll_max_timeout = 100000;
static int constexpr g_epoll_max_events = 10;

EventLoop::EventLoop() {
    if (t_current_eventloop) {
        ERRORLOG("failed to create event loop, alread exist");
        exit(0);
    }
    m_thread_id = getThreadId();
    m_epoll_fd = epoll_create(10);

    if (m_epoll_fd == -1) {
        ERRORLOG("faile to create epoll.");
        exit(0);
    }
    initWakeUpFdEvent();
    initTimer();
    INFOLOG("create event loop in thread %d", m_thread_id);
    t_current_eventloop = this;
}

void EventLoop::addTimerEvent(TimerEvent::s_ptr event) {
    m_timer->addTimerEvent(event);
}

void EventLoop::initTimer() {
    m_timer = new Timer();
    addEpollEvent(m_timer);
}

void EventLoop::initWakeUpFdEvent() {
    m_wakeup_fd = eventfd(0, EFD_NONBLOCK );
    if (m_wakeup_fd < 0) {
        ERRORLOG("failed to create eventfd");
        exit(0);
    }
    m_wakeup_fd_event = new WakeUpFdEvent(m_wakeup_fd);

    m_wakeup_fd_event->listen(FdEvent::IN_EVENT, [this](){
            char buf[8];
            while (read(m_wakeup_fd, buf, sizeof(buf)) != -1 && errno != EAGAIN) {
                DEBUGLOG("read wake up");
            }
            DEBUGLOG("read full bytes from wake up fd:%d", m_wakeup_fd);
    });

    addEpollEvent(m_wakeup_fd_event);

}

EventLoop::~EventLoop() {
    close(m_epoll_fd);
    delete m_wakeup_fd_event;
    delete m_timer;
}

void EventLoop::loop() {
    m_is_looping = true;
    while (!m_stop_flag) {
        queue<function<void()>> tmp;
        {
        std::lock_guard<mutex> lk{m_mut}; 
        m_pending_tasks.swap(tmp);
        }
        while (!tmp.empty()) {
            auto cb = tmp.front();
            tmp.pop();
            if (cb) {cb();}
            DEBUGLOG("exec cb");
        }
        int timeout = g_epoll_max_timeout;
        epoll_event result_events[g_epoll_max_events];

        int rt = epoll_wait(m_epoll_fd, result_events, g_epoll_max_events, timeout );
        if (rt < 0) {
            ERRORLOG("epoll wait error, error=%d", errno);
        } else {
            DEBUGLOG("get %d event.",rt);
            for (int i = 0; i < rt; i++) {
                epoll_event trigger_event = result_events[i];
                FdEvent* fd_event = static_cast<FdEvent*>(trigger_event.data.ptr);
                if (!fd_event) {
                    DEBUGLOG("fd event null");
                    continue;
                }
                if (trigger_event.events & EPOLLIN) {
                    DEBUGLOG("get epollin event");
                    addTask(fd_event->handler(FdEvent::IN_EVENT));
                }
                if (trigger_event.events & EPOLLOUT) {
                    DEBUGLOG("get epollout event");
                    addTask(fd_event->handler(FdEvent::OUT_EVENT));
                }
                if (trigger_event.events & EPOLLERR) {
                    DEBUGLOG("get error events");
                }
                uint32_t a = trigger_event.events;
                DEBUGLOG("event vals:%u, fd:%d", a, fd_event->getFd());
            }
        }
    }

}

void EventLoop::wakeup() {
    DEBUGLOG("wakeup");
    m_wakeup_fd_event->wakeup();
}


void EventLoop::stop() {
    m_stop_flag = true;
}

void EventLoop::addEpollEvent(FdEvent* event) {
    if (isInLoopThread()) {
        ADD_TO_EPOLL();
        //auto it = m_listen_fds.find(event->getFd());
        //int op = EPOLL_CTL_ADD;
        //if (it != m_listen_fds.end()) {
            //op = EPOLL_CTL_ADD;
        //}
        //epoll_event tmp = event->getEpollEvent();
        //int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp);
        //if (rt == 0) {
            //ERRORLOG("failed to add epoll event");
        //}
    } else {
        auto cb = [this, event] () {
            ADD_TO_EPOLL();
        };
        addTask(cb, true);

    }
}

void EventLoop::delEpollEvent(FdEvent* event) {
    if (isInLoopThread()) {
        DEL_TO_EPOLL();         
    } else {
        auto cb = [this, event] () {
            DEL_TO_EPOLL();
        };
        addTask(cb, true);
    }
}

void EventLoop::addTask(std::function<void()> cb, bool is_wake_up) {
    std::lock_guard<mutex> lk{m_mut};
    m_pending_tasks.push(cb);
    if (is_wake_up) {
        DEBUGLOG("wakeup");
        wakeup();
    }
    DEBUGLOG("add a task end");
}

bool EventLoop::isInLoopThread() {
    return getThreadId() == m_thread_id;
}
EventLoop* EventLoop::GetCurrentEventLoop() {
    if (t_current_eventloop) {
        return t_current_eventloop;
    }
    t_current_eventloop = new EventLoop();
    return t_current_eventloop;
}

}