#pragma once
#include "abstract_codec.h"
#include "abstract_protocol.h"
#include "codec/tinypb_protocol.h"
#include <memory>
namespace rocket {
class TinyPBCodec : public AbstractCodec {
public:
    void encode(std::vector<AbstractProtocol::s_ptr> &msgs, TcpBuffer::s_ptr out_buffer) override;

    void decode(std::vector<AbstractProtocol::s_ptr> &msgs, TcpBuffer::s_ptr buffer) override;

    virtual ~TinyPBCodec(){}
private:
    const char* encodeTinyPB(std::shared_ptr<TinyPBProtocol> msg, int &len);
};
}