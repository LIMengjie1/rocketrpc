#pragma once

#include "god.h"
#include "log.h"
#include <functional>
#include <sys/epoll.h>
#include <fcntl.h>
#include <system_error>

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

    void cancle(TriggerEvent);

    int getFd() const {
        return m_fd;
    }

    epoll_event getEpollEvent() {
        return m_listen_events;
    }

    void setNonBlock() {
       DEBUGLOG("fd;%d", m_fd);
       int flag = fcntl(m_fd, F_GETFL, 0); 
       if(flag & O_NONBLOCK) {
        return;
       }
       fcntl(m_fd, F_SETFL, flag | O_NONBLOCK);
    }

protected:
    int m_fd{-1};
    epoll_event m_listen_events;
    std::function<void()> m_read_callback;
    std::function<void()> m_write_calback;
};
}