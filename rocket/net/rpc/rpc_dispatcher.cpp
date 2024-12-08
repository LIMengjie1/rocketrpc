#include "rpc_dispatcher.h"
#include "error_code.h"
#include "log.h"
#include "net_addr.h"
#include "protobuf/descriptor.h"
#include "protobuf/message.h"
#include "rpc_controller.h"
#include "tcp_connection.h"
#include "tinypb_protocol.h"
#include <memory>
#include <string>
#include <system_error>

namespace rocket {

static RpcDispatcher* g_rpc_dispatcher = nullptr;

RpcDispatcher* RpcDispatcher::GetRpcDispatcher(){
    if (g_rpc_dispatcher) return g_rpc_dispatcher;

    g_rpc_dispatcher = new RpcDispatcher;
    return g_rpc_dispatcher;
}

void RpcDispatcher::dispatch(AbstractProtocol::s_ptr request, AbstractProtocol::s_ptr response, TcpConnection* connection) {
    std::shared_ptr<TinyPBProtocol> req_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(request);
    std::shared_ptr<TinyPBProtocol> rsp_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(response);
    std::string method_full_name = req_protocol->m_method_name;
    std::string service_name;
    std::string method_name;

    rsp_protocol->m_req_id = req_protocol->m_req_id;
    DEBUGLOG("get req id:%s", req_protocol->m_req_id.c_str());
    rsp_protocol->m_method_name = req_protocol->m_method_name;
    if (!parseServiceFullName(method_full_name, service_name, method_name)) {
        ERRORLOG("%s parser full name failed", rsp_protocol->m_req_id.c_str());
        setTinyPBError(rsp_protocol, ERROR_PARSE_SERVICE_NAME, "parse service name error");
        return;
    }
    auto it = m_service_map.find(service_name);
    if (it == m_service_map.end()) {
        ERRORLOG("%s service name:%s not found", rsp_protocol->m_req_id.c_str(), service_name.c_str());
        setTinyPBError(rsp_protocol, ERROR_SERVICE_NOT_FOUND, "service not found");
        return;

    }
    service_s_ptr service = it->second;
    const google::protobuf::MethodDescriptor* method = service->GetDescriptor()->FindMethodByName(method_name);
    if (method == nullptr) {
        ERRORLOG("%s method name:%s not found", rsp_protocol->m_req_id.c_str(), method_name.c_str());
        setTinyPBError(rsp_protocol, ERROR_SERVICE_NOT_FOUND, "method not found");
        return;
    }
    google::protobuf::Message* req_msg = service->GetRequestPrototype(method).New();
    
    if(!req_msg->ParseFromString(req_protocol->m_pb_data)) {
        ERRORLOG("%s | deseilize error",rsp_protocol->m_req_id.c_str());
        setTinyPBError(rsp_protocol, ERROR_FAILED_DESERIALIZE, "deseilize error");
        delete req_msg;
        return;
    }
    INFOLOG("req id [%s], get rpc request %s", req_protocol->m_req_id.c_str(), req_msg->ShortDebugString().c_str());

    google::protobuf::Message* rsp_msg = service->GetRequestPrototype(method).New();
    
    RpcController rpcController;
    rpcController.SetLocalAddr(connection->getLocalAddr());
    rpcController.SetPeerAddr(connection->getPeerAddr());
    rpcController.SetReqId(req_protocol->m_req_id);
    service->CallMethod(method, &rpcController, req_msg, rsp_msg, nullptr);

    if (!rsp_msg->SerializeToString(&rsp_protocol->m_pb_data)) {
        ERRORLOG("%s | seilize error",rsp_protocol->m_req_id.c_str());
        setTinyPBError(rsp_protocol, ERROR_FAILED_SERIALIZE, "serilize error");
        delete req_msg;
        delete rsp_msg;
        return;
    }

    rsp_protocol->m_err_code = 0;
    INFOLOG("%s | dispatch succ, request[%s], response[%s]", req_protocol->m_req_id.c_str(), req_msg->ShortDebugString().c_str(), rsp_msg->ShortDebugString().c_str());
    delete req_msg;
    delete rsp_msg;
}

void RpcDispatcher::registerService(service_s_ptr service) {
    std::string service_name = service->GetDescriptor()->full_name();
    m_service_map[service_name] = service;
}


bool RpcDispatcher::parseServiceFullName(const std::string& full_name, std::string& service_name, std::string& method_name) {
    if(full_name.empty()) return false;

    size_t i = full_name.find_first_of(".");
    if (i == full_name.npos) {
        ERRORLOG("not find . in full name[%s]", full_name);
        return false;
    }

    service_name = full_name.substr(0, i);
    method_name = full_name.substr(i + 1, full_name.length() -i - 1);
    INFOLOG("parser service name:%s, method name:%s, full_name:%s", service_name.c_str(), method_name.c_str(),full_name.c_str());
    return true;
}
void RpcDispatcher::setTinyPBError(std::shared_ptr<TinyPBProtocol> msg, int32_t err_code, const std::string& err_info) {
    msg->m_err_code = err_code;
    msg->m_err_info = err_info;
    msg->m_err_info_len = err_info.length();
}
}