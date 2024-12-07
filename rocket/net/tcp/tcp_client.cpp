
#include "tcp_client.h"
#include "eventloop.h"
#include "fd_event.h"
#include "fd_event_group.h"
#include "log.h"
#include "tcp_connection.h"
#include <asm-generic/socket.h>
#include <cerrno>
#include <cstring>
#include <memory>
#include <sys/socket.h>
namespace rocket {

TcpClient::TcpClient(NetAddr::s_ptr peer_addr) {
    m_peer_addr = peer_addr;
    m_event_loop = EventLoop::GetCurrentEventLoop();
    m_fd = ::socket(peer_addr->getFamily(), SOCK_STREAM, 0);

    if (m_fd < 0) {
        ERRORLOG("TcpClient failed to create fd");
    }

    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(m_fd);
    m_fd_event->setNonBlock();

    m_connection = std::make_shared<TcpConnection>(m_event_loop, m_fd, 128, peer_addr, TcpConnection::TcpConnectionByClient);
    m_connection->setConnectionType(TcpConnection::TcpConnectionByClient);
}

TcpClient::~TcpClient() {
    if (m_fd > 0) {
        close(m_fd);
    }

}

void TcpClient::connect(std::function<void()> done) {
    int ret = ::connect(m_fd, m_peer_addr->getSockAddr(), m_peer_addr->getSockLen());
    if (ret == 0) {
        DEBUGLOG("connect succ");
        if (done) {
            done();
        }
    } else if (ret == -1) {
        if (errno == EINPROGRESS) {
            m_fd_event->listen(FdEvent::OUT_EVENT, [this, done](){
                int error = 0;
                socklen_t err_len = sizeof(error);
                getsockopt(m_fd, SOL_SOCKET, SO_ERROR, &error, &err_len);
                bool is_connect_succ =false;
                if (error == 0) {
                    DEBUGLOG("connect to %s succ", m_peer_addr->toString().c_str());
                    is_connect_succ = true;
                    m_connection->setState(TcpConnection::Connected);
                } else {
                    DEBUGLOG("connect to %s failed", m_peer_addr->toString().c_str());
                }
                m_fd_event->cancle(FdEvent::OUT_EVENT);
                m_event_loop->delEpollEvent(m_fd_event);
                if (is_connect_succ && done) {
                    done();
                } 
            });
            m_event_loop->addEpollEvent(m_fd_event);
            if (!m_event_loop->isLooping()) {
                m_event_loop->loop();
            }
        } else {
            ERRORLOG("connect error, errno=%d, err=%s", errno, strerror(errno));
        }

    }

}

void TcpClient::writeMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done) {
   m_connection->pushSendMsg(message, done);
   m_connection->listenWrite();
}

void TcpClient::readMessage(const string& req_id, std::function<void(AbstractProtocol::s_ptr)> done) {
    m_connection->pushReadMsg(req_id, done);
    m_connection->listenRead();
}
}