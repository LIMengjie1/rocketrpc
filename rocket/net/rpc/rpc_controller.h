#pragma once
#include "net_addr.h"
#include "protobuf/stubs/callback.h"
#include "rpc_dispatcher.h"
#include <google/protobuf/service.h>
#include <string>

namespace rocket {

class RpcController : public google::protobuf::RpcController {
public:
    RpcController(){}
    ~RpcController(){}

    void Reset() override;

    bool Failed()const override;

    std::string ErrorText() const override;

    void StartCancel() override;

    void SetFailed(const std::string& reason) override;

    bool IsCanceled() const override;

    void NotifyOnCancel(google::protobuf::Closure* callback) override;

    void SetError(int32_t error_code, const std::string error_info);

    int32_t GetErrorCode();

    std::string GetErrorInfo();

    void SetReqId(const std::string& msg_id);

    std::string GetReqId();

    void SetLocalAddr(NetAddr::s_ptr addr);

    void SetPeerAddr(NetAddr::s_ptr addr);

    NetAddr::s_ptr GetLocalAddr();

    NetAddr::s_ptr GetPeerAddr();

    void SetTimeout(int timeout);

    int GetTimeout();

    bool Finished();

    void SetFinished(bool value);

private:
    int32_t m_error_code = 0;
    std::string m_error_info;
    std::string m_req_id;

    bool m_is_failed = false;
    bool m_is_cancled = false;
    bool m_is_finished = false;

    NetAddr::s_ptr m_local_addr;
    NetAddr::s_ptr m_peer_addr;

    int m_timeout = 1000;
};

}