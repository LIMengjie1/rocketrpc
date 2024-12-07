#pragma once
#include "abstract_protocol.h"
#include "eventloop.h"
#include "fd_event.h"
#include "net_addr.h"
#include "tcp_connection.h"
#include <functional>

namespace rocket {
class TcpClient {
public:
    TcpClient(NetAddr::s_ptr peer_addr);

    ~TcpClient();

    void connect(std::function<void()>);

    void writeMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done);

    void readMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done);

private:
    NetAddr::s_ptr m_peer_addr;
    EventLoop* m_event_loop = nullptr;

    int m_fd = -1;
    FdEvent* m_fd_event = nullptr;
    TcpConnection::s_ptr m_connection;
};

}