#include "tcp_connection.h"
#include "eventloop.h"
#include "fd_event.h"
#include "tcpbuffer.h"
#include "fd_event_group.h"
#include "log.h"
#include <cerrno>
#include <memory>
#include <sys/socket.h>
#include <vector>

namespace rocket {

TcpConnection::TcpConnection(EventLoop* eventloop, int fd, int buffer_size, NetAddr::s_ptr peer_addr) {
    m_peer_addr = peer_addr;
    m_state = NotConnected;
    m_eventloop = eventloop;

    m_in_buffer = std::make_shared<TcpBuffer>(buffer_size);
    m_out_buffer = std::make_shared<TcpBuffer>(buffer_size);

    m_fd_event =  FdEventGroup::GetFdEventGroup()->getFdEvent(fd);
    m_fd_event->setNonBlock();

    m_fd_event->listen(FdEvent::IN_EVENT, [this]() {
        this->read();
    });
    m_eventloop->addEpollEvent( m_fd_event);
}

TcpConnection::~TcpConnection(){
    DEBUGLOG("tcp connection dtor");

}

void TcpConnection::read() {
    if (m_state != Connected) {
        INFOLOG("read from not connect fd, addr[%s], clientfd[%d]", m_peer_addr->toString().c_str(), m_fd_event->getFd());
        return;
    }
    bool is_read_all = false;
    bool is_close = false;

    while (!is_read_all) {
        if (m_in_buffer->writeAble() == 0) {
            m_in_buffer->resizeBuffer(2 * m_in_buffer->getBuffer().size());
        }
        int read_count = m_in_buffer->writeAble();
        int write_index = m_in_buffer->writeIndex();

        int rt = ::read(m_fd_event->getFd(), &(m_in_buffer->getBuffer()[write_index]), read_count);
        DEBUGLOG("read %d bytes from addr[%s], client fd:%d",rt, m_peer_addr->toString().c_str(), m_fd_event->getFd());

        if (rt > 0) {
            m_in_buffer->moveWriteIndex(rt);
            if (rt == read_count) {
                continue;
            } else if (rt < read_count) {
                is_read_all = true;
                break;
            }
        } else if(rt == 0) {
            is_close = true;
            break;
        } else if (rt == -1 && errno == EAGAIN) {
            is_read_all = true;
            break;
        }
    }

    if (is_close) {
        clear();
        is_read_all = true;
        INFOLOG("peer closed peer addr[%s], client fd[%d]", m_peer_addr->toString().c_str(), m_fd_event->getFd());
    }
    if (!is_read_all) {
        ERRORLOG("not read all");
    }
    excute();
}

void TcpConnection::clear() {
    if (m_state == TcpState::Closed) {
        return;
    }
    m_fd_event->cancle(FdEvent::IN_EVENT);
    m_fd_event->cancle(FdEvent::OUT_EVENT);

    m_eventloop->delEpollEvent(m_fd_event);
    m_state = TcpState::Closed;
}

void TcpConnection::excute() {
    if (m_state == Closed) return;

    std::vector<char> tmp;
    int size = m_in_buffer->readAble();
    tmp.resize(size);
    m_in_buffer->readToBuffer(tmp, size);
    string str{tmp.begin(), tmp.end()};
    str += '\0';

    INFOLOG("succ get request[%s] from client[%s]",str.c_str(), m_peer_addr->toString().c_str());

    string str2("fuck");
    //m_out_buffer->writeToBuffer(tmp.data(), tmp.size());
    m_out_buffer->writeToBuffer(str2.c_str(), str2.length());

    m_fd_event->listen(FdEvent::OUT_EVENT, [this](){
        this->write();
    });
    m_eventloop->addEpollEvent(m_fd_event);
}

void TcpConnection::write() {
    if (m_state != Connected) {
        ERRORLOG("write to not connect fd, addr[%s], clientfd[%d]", m_peer_addr->toString().c_str(), m_fd_event->getFd());
    }

    bool is_write_all = false;
    while(true) {
        if (m_out_buffer->readAble() == 0) {
            DEBUGLOG("no data need to send to client[%s]", m_peer_addr->toString().c_str());
            is_write_all = true;
            break;
        }

        int write_size = m_out_buffer->readAble();
        int read_index = m_out_buffer->readIndex();

        int rt = ::write(m_fd_event->getFd(), &m_out_buffer->getBuffer()[read_index], write_size);
        DEBUGLOG("write msg:" );

        if (rt >= write_size) {
            is_write_all = true;
            DEBUGLOG("no data need to sedn to client[%s]", m_peer_addr->toString().c_str());
            break;
        } if (rt == -1 && errno == EAGAIN) {
            ERRORLOG("write data error, errno = eagain, and rt=-1 client[%s]", m_peer_addr->toString().c_str());
            break;
        }
    }

    if (is_write_all) {
        m_fd_event->cancle(FdEvent::OUT_EVENT);
        m_eventloop->addEpollEvent(m_fd_event);
    }
}

 void TcpConnection::setState(TcpConnection::TcpState state) {
     m_state = state;
 }

TcpConnection::TcpState TcpConnection::getState() {
    return m_state;
}

void TcpConnection::shutdown() {
    if (m_state == TcpState::Closed || m_state == TcpState::NotConnected) return;

    m_state = TcpState::HalfClosing;

    // 当FD发生可读事件  但是可读数据为0 即对端发送了FIN
    ::shutdown(m_fd_event->getFd(), SHUT_RDWR);
}
}