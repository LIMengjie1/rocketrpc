#pragma once
#include "fd_event.h"
#include "god.h"
#include "tcp_acceptor.h"
#include "net_addr.h"
#include "eventloop.h"
#include "io_thread_group.h"

namespace rocket {
class TcpServer {
public:
    TcpServer(NetAddr::s_ptr);

    ~TcpServer();

    void start();

    void init();

    void onAccept();

private:
    TcpAcceptor::s_ptr m_acceptor;
    NetAddr::s_ptr m_local_addr;
    EventLoop* m_main_event_loop = nullptr;
    IOThreadGroup* m_io_thread_group = nullptr;
    FdEvent* m_listen_fd_event;
    int m_client_counts = 0;
};

}