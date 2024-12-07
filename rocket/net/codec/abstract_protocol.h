#pragma once

#include <memory>

namespace rocket {
class AbstractProtocol : public std::enable_shared_from_this<AbstractProtocol>{
public:
    using s_ptr = std::shared_ptr<AbstractProtocol>;
    std::string getReqId() {
        return m_req_id;
    }

    void setReqId(const std::string& id) {
        m_req_id = id;
    }

    virtual ~AbstractProtocol(){};

    std::string m_req_id;
};
}