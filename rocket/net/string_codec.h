#pragma once

#include "abstract_codec.h"
#include <memory>
#include <vector>
namespace rocket {

class StringProtocol : public AbstractProtocol {
public:
    string info;
};

class StringCodec : public AbstractCodec {

    void encode(std::vector<AbstractProtocol::s_ptr> &msgs, TcpBuffer::s_ptr out_buffer) override{
        for (auto& v: msgs) {
            std::shared_ptr<StringProtocol> msg = std::dynamic_pointer_cast<StringProtocol>(v);
            out_buffer->writeToBuffer(msg->info.c_str(), msg->info.size());
        }
    }

    void decode(std::vector<AbstractProtocol::s_ptr> &msgs, TcpBuffer::s_ptr buffer) override {

        std::vector<char> ret;
        buffer->readToBuffer(ret, buffer->readAble());

        string info;
        for (int i = 0; i < ret.size(); i++) {
            info += ret[i];
        }

        std::shared_ptr<StringProtocol> msg = std::make_shared<StringProtocol>();
        msg->info = info;
        msg->setReqId("123");
        msgs.push_back(msg);
    }
};

}