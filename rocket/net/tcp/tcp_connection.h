#pragma once
#include "fd_event.h"
#include "god.h"
#include "net_addr.h"
#include "tcpbuffer.h"
#include "io_thread.h"

namespace rocket {
class TcpConnection {
public:
    public: 
    enum TcpState {
        NotConnected = 1,
        Connected = 2,
        HalfClosing = 3,
        Closed = 4
    };
    TcpConnection(IOThread*, int fd, int buffer_size, NetAddr::s_ptr peer_addr);

    ~TcpConnection();

    void read();

    void excute();

    void write();

private:
    NetAddr::s_ptr m_local_addr;
    NetAddr::s_ptr m_peer_addr;

    TcpBuffer::s_ptr m_in_buffer;
    TcpBuffer::s_ptr m_out_buffer;

    IOThread* m_io_thread = nullptr;
    FdEvent* m_fd_event = nullptr;

    TcpState m_state;
};

}