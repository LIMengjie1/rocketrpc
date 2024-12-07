#pragma once
#include "eventloop.h"
#include "fd_event.h"
#include "god.h"
#include "net_addr.h"
#include "tcpbuffer.h"
#include "io_thread.h"
#include <memory>

namespace rocket {
class TcpConnection {
public:
    using s_ptr = std::shared_ptr<TcpConnection>;
    enum TcpState {
        NotConnected = 1,
        Connected = 2,
        HalfClosing = 3,
        Closed = 4
    };

    enum TcpConnectionType {
        TcpConnectionByServer = 1,
        TcpConnectionByClient = 2
    };

    TcpConnection(EventLoop*, int fd, int buffer_size, NetAddr::s_ptr peer_addr);

    ~TcpConnection();

    void read();

    void excute();

    void write();

    void setState(TcpConnection::TcpState);

    TcpState getState();

    void clear();

    void shutdown();

    void setConnectionType(TcpConnectionType type) {
        m_connection_type = type;
    }

private:
    NetAddr::s_ptr m_local_addr;
    NetAddr::s_ptr m_peer_addr;

    TcpBuffer::s_ptr m_in_buffer;
    TcpBuffer::s_ptr m_out_buffer;

    FdEvent* m_fd_event = nullptr;
    EventLoop* m_eventloop = nullptr;

    TcpState m_state;
    TcpConnectionType m_connection_type = TcpConnectionByServer;

};

}