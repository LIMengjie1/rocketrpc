#pragma once

#include "god.h"
#include <functional>
#include <sys/epoll.h>

namespace rocket {
class FdEvent {
public:
    enum TriggerEvent{
       IN_EVENT = EPOLLIN,
       OUT_EVENT = EPOLLOUT, 
    };

    FdEvent(int fd);
    FdEvent() = default;

    ~FdEvent();

    std::function<void()> handler(TriggerEvent );

    void listen(TriggerEvent, std::function<void()>);

    int getFd() const {
        return m_fd;
    }

    epoll_event getEpollEvent() {
        return m_listen_events;
    }

protected:
    int m_fd{-1};
    epoll_event m_listen_events;
    std::function<void()> m_read_callback;
    std::function<void()> m_write_calback;
};
}