#include "fd_event.h"
#include "log.h"
#include <functional>
#include <sys/epoll.h>

namespace rocket {
FdEvent::FdEvent(int fd) : m_fd(fd) {
    memset(&m_listen_events, 0, sizeof(m_listen_events));
}

FdEvent::~FdEvent() {

}

std::function<void()> FdEvent::handler(TriggerEvent event) {
    if (event == TriggerEvent::IN_EVENT) {
        return m_read_callback;
    } else {
        return m_write_calback;
    }
}

void FdEvent::listen(TriggerEvent event_type, std::function<void()> callback) {
    if (event_type == TriggerEvent::IN_EVENT) {
        m_listen_events.events |= EPOLLIN;
        m_read_callback = callback;
        m_listen_events.data.ptr = this;

    } else {
        m_listen_events.events |= EPOLLOUT;
        m_write_calback = callback;
        m_listen_events.data.ptr = this;
    }
}

void FdEvent::cancle(TriggerEvent e) {
    if (e == TriggerEvent::IN_EVENT) {
        m_listen_events.events &= (~EPOLLIN);
    } else  {
        m_listen_events.events &= (~EPOLLOUT);
    }
}
}