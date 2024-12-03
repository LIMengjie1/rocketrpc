#include "tcp_connection.h"
#include "fd_event.h"
#include "tcpbuffer.h"
#include "fd_event_group.h"
#include "log.h"
#include <cerrno>
#include <memory>
#include <vector>

namespace rocket {

TcpConnection::TcpConnection(IOThread* io_thread, int fd, int buffer_size, NetAddr::s_ptr peer_addr) {
    m_io_thread = io_thread;
    m_peer_addr = peer_addr;
    m_state = NotConnected;

    m_in_buffer = std::make_shared<TcpBuffer>(buffer_size);
    m_out_buffer = std::make_shared<TcpBuffer>(buffer_size);

    m_fd_event =  FdEventGroup::GetFdEventGroup()->getFdEvent(fd);
    m_fd_event->setNonBlock();

    m_fd_event->listen(FdEvent::IN_EVENT, [this]() {
        this->read();
    });
}

TcpConnection::~TcpConnection(){

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
        DEBUGLOG("read %d bytes from addr[%s], client fd:%d", m_peer_addr->toString().c_str(), m_fd_event->getFd());

        if (rt > 0) {
            m_in_buffer->moveWriteIndex(rt);
            if (rt == read_count) {
                continue;
            } else if (rt < read_count) {
                is_read_all = true;
                break;
            }
            
        } else {
            is_close = true;
        }
    }

    if (is_close) {
        INFOLOG("peer closed peer addr[%s], client fd[%d]", m_peer_addr->toString().c_str(), m_fd_event->getFd());
    }
    if (!is_read_all) {
        ERRORLOG("not read all");
    }

    excute();
}

void TcpConnection::excute() {
    std::vector<char> tmp;
    int size = m_in_buffer->readAble();
    tmp.resize(size);
    m_in_buffer->readToBuffer(tmp, size);

    INFOLOG("succ get request from client[%s]", m_peer_addr->toString().c_str());


    m_out_buffer->writeToBuffer(tmp.data(), tmp.size());

    m_fd_event->listen(FdEvent::OUT_EVENT, [this](){
        this->write();
    });
}

void TcpConnection::write() {
    if (m_state != Connected) {
        ERRORLOG("write to not connect fd, addr[%s], clientfd[%d]", m_peer_addr->toString().c_str(), m_fd_event->getFd());
    }

    while(true) {
        if (m_out_buffer->readAble() == 0) {
            DEBUGLOG("no data need to sedn to client[%s]", m_peer_addr->toString().c_str());
            break;
        }

        int write_size = m_out_buffer->readAble();
        int read_index = m_out_buffer->readIndex();

        int rt = ::write(m_fd_event->getFd(), &m_out_buffer->getBuffer()[read_index], write_size);

        if (rt >= write_size) {
            DEBUGLOG("no data need to sedn to client[%s]", m_peer_addr->toString().c_str());
            break;
        } if (rt == -1 && errno == EAGAIN) {
            ERRORLOG("write data error, errno = eagain, and rt=-1 client[%s]", m_peer_addr->toString().c_str());
            break;
        }
    }
}
}