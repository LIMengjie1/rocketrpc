#include "tcp_server.h"
#include "eventloop.h"
#include "fd_event.h"
#include "io_thread.h"
#include "io_thread_group.h"
#include "log.h"
#include "net_addr.h"
#include "tcp_acceptor.h"
#include "tcp_connection.h"
#include <iterator>
#include <memory>

namespace rocket {

TcpServer::TcpServer(NetAddr::s_ptr local_addr) : m_local_addr(local_addr) {
    init();
    INFOLOG("rocket rpc tcpserver start succ, listen o [%s]", m_local_addr->toString().c_str());
}

TcpServer::~TcpServer() {
    delete m_io_thread_group;
    delete m_main_event_loop;
    m_io_thread_group = nullptr;
    m_main_event_loop = nullptr;
}

void TcpServer::start() {
    m_io_thread_group->start();
    m_main_event_loop->loop();
}

void TcpServer::init() {
    m_acceptor = std::make_shared<TcpAcceptor>(m_local_addr);
    m_main_event_loop = EventLoop::GetCurrentEventLoop();
    m_io_thread_group = new IOThreadGroup(2);

    m_listen_fd_event = new FdEvent(m_acceptor->getListenFd());
    m_listen_fd_event->listen(FdEvent::IN_EVENT, [this](){
        onAccept();
    });
    m_main_event_loop->addEpollEvent(m_listen_fd_event);
}

void TcpServer::onAccept() {
    auto ret = m_acceptor->accept();
    int client_fd = ret.first;
    NetAddr::s_ptr  peer_addr = ret.second;
    m_client_counts++;
    //TODO client fd 添加到任意IOthread
    //m_io_thread_group->getIOThread()->getEventLoop()->addEpollEvent(FdEvent *event);
    IOThread* io_thread = m_io_thread_group->getIOThread();
    TcpConnection::s_ptr  connection = make_shared<TcpConnection>(io_thread->getEventLoop(), client_fd, 128, peer_addr);
    connection->setState(TcpConnection::Connected);
    m_client.insert(connection);
    INFOLOG("tcp server succ get client fd:%d", client_fd);
}

}