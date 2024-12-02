#pragma once
#include "rocket/common/god.h"
#include "log.h"
#include "net_addr.h"
#include <memory>
#include <sys/socket.h>

namespace rocket {
class TcpAcceptor {
public:
    TcpAcceptor(NetAddr::s_ptr local_addr);

    ~TcpAcceptor();

    int accept() {
        if (m_family == AF_INET) {
            sockaddr_in client_addr;
            memset(&client_addr, 0, sizeof(client_addr));
            socklen_t client_addr_len = sizeof(client_addr);

            int rt = ::accept(m_listenfd, reinterpret_cast<sockaddr*>(&client_addr), &client_addr_len);
            if (rt < 0) {
                ERRORLOG("accept new client failed, erno:%d, error:%s", errno, strerror(errno));
            }
            IPNetAddr peer_addr(client_addr);
            INFOLOG("a client have accepted succ, peer addr [%s]", peer_addr.toString());
            return rt;
        }
    }

private:
    NetAddr::s_ptr  m_local_addr;
    int m_family{-1};
    int m_listenfd = -1;
};
}