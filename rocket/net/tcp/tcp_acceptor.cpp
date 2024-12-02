#include "tcp_acceptor.h"
#include "log.h"
#include "net_addr.h"
#include "tcpbuffer.h"
#include <asm-generic/socket.h>
#include <cassert>
#include <sys/socket.h>

namespace rocket {
TcpAcceptor::TcpAcceptor(NetAddr::s_ptr local_addr) {
    m_local_addr = local_addr;
    if (!local_addr->checkValid()) {
        ERRORLOG("invalid local addr %s", local_addr->toString().c_str());
        exit(0);
    }

    m_family = m_local_addr->getFamily();
    m_listenfd = socket(m_family, SOCK_STREAM, 0);
    if (m_listenfd < 0) {
        ERRORLOG("listen fd create failed");
        exit(0);
    }
    
    int val = -1;
    if (setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) != 0) {
        ERRORLOG("set socket opt reuse addr failed, errno=%d, error=%s", errno, strerror(errno))
    }
    socklen_t len = m_local_addr->getSockLen();
    if (bind(m_listenfd, m_local_addr->getSockAddr(), len) != 0) {
        ERRORLOG("bind listen fd failed");
        exit(0);
    }
    if (listen(m_listenfd, 100) !=0 ) {
        ERRORLOG("listen fd failed");
        exit(0);
    }
}

TcpAcceptor::~TcpAcceptor() {

}
}