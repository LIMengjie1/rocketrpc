#pragma once
#include "codec/abstract_protocol.h"
#include "tinypb_protocol.h"
#include <map>
#include <memory>
#include <string>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

namespace rocket {

class TcpConnection;

class RpcDispatcher {
public:
    static RpcDispatcher* GetRpcDispatcher();
    using service_s_ptr = std::shared_ptr<google::protobuf::Service>;
    void dispatch(AbstractProtocol::s_ptr request, AbstractProtocol::s_ptr response, TcpConnection*);

    void registerService(service_s_ptr);

    void setTinyPBError(std::shared_ptr<TinyPBProtocol> msg, int32_t err_code, const std::string& err_info);

private:
    bool parseServiceFullName(const std::string& full_name, std::string& service_name, std::string& method_name);
    std::map<std::string, service_s_ptr> m_service_map;
};

}