#pragma once
#include "codec/abstract_protocol.h"
#include "eventloop.h"
#include "fd_event.h"
#include "god.h"
#include "net_addr.h"
#include "rpc_dispatcher.h"
#include "tcpbuffer.h"
#include "io_thread.h"
#include "codec/abstract_codec.h"
#include <functional>
#include <memory>
#include <vector>

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

    TcpConnection(EventLoop*, int fd, int buffer_size, NetAddr::s_ptr peer_addr, NetAddr::s_ptr local_addr, TcpConnectionType type = TcpConnectionByServer);

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

    void listenWrite();

    void listenRead();

    void pushSendMsg(AbstractProtocol::s_ptr, std::function<void(AbstractProtocol::s_ptr)>);

    void pushReadMsg(const string& req_id, std::function<void(AbstractProtocol::s_ptr)>);

    NetAddr::s_ptr getLocalAddr() {
        return m_local_addr;
    }

    NetAddr::s_ptr getPeerAddr() {
        return m_peer_addr;
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

    std::vector<std::pair<AbstractProtocol::s_ptr, std::function<void(AbstractProtocol::s_ptr)>>> m_write_done_callback;
    std::map<string, std::function<void(AbstractProtocol::s_ptr)>> m_read_done_callback;
    AbstractCodec* m_codec;
};

}