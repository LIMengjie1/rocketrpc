#pragma once
#include "tcpbuffer.h"
#include "abstract_protocol.h"
#include <vector>

namespace rocket {
class AbstractCodec {
public:
    virtual void encode(std::vector<AbstractProtocol::s_ptr> &msgs, TcpBuffer::s_ptr out_buffer) = 0;

    virtual void decode(std::vector<AbstractProtocol::s_ptr> &msgs, TcpBuffer::s_ptr buffer) = 0;

    virtual ~AbstractCodec(){};
};
}