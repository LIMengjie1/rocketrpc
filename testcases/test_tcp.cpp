
#include "net_addr.h"
#include "rocket/common/log.h"
#include "rocket/common/config.h"
#include "rocket/net/fd_event.h"
#include "rocket/net/eventloop.h"
#include "rocket/net/timer_event.h"
#include "rocket/net/io_thread.h"
#include "rocket/net/io_thread_group.h"
#include "rocket/net/tcp/tcp_server.h"
#include <iostream>
#include <memory>
#include <sys/socket.h>

void test_tcp_server() {
    rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 12345);
    DEBUGLOG("get addr%s", addr->toString().c_str());

    rocket::TcpServer tcp_server(addr);
    tcp_server.start();
}

int main() {
    rocket::Config::SetGlobalConfig("../conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();
    test_tcp_server();
}