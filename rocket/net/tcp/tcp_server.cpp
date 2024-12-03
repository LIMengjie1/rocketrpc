#include "tcp_server.h"
#include "eventloop.h"
#include "fd_event.h"
#include "io_thread_group.h"
#include "log.h"
#include "net_addr.h"
#include "tcp_acceptor.h"
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
    int client_fd = m_acceptor->accept();
    m_client_counts++;
    //TODO client fd 添加到任意IOthread
    //m_io_thread_group->getIOThread()->getEventLoop()->addEpollEvent(FdEvent *event);
    INFOLOG("tcp server succ get client fd:%d", client_fd);
}

}