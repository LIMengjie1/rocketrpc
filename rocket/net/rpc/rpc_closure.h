#pragma once
#include <google/protobuf/stubs/callback.h>
#include <functional>
#include <memory>
#include "rocket/common/run_time.h"
#include "rocket/common/log.h"
#include "rocket/common/exception.h"
#include "rocket/net/rpc/rpc_interface.h"

namespace rocket {

class RpcClosure : public google::protobuf::Closure {
 public:
    void Run() override {
        if (!m_cb) {
            m_cb();
        }
    }
 private:
  std::function<void()> m_cb {nullptr};

};

}