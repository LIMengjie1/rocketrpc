#include "tinypb_codec.h"
#include "codec/tinypb_protocol.h"
#include "log.h"
#include "util.h"
#include <cstdlib>
#include <cstring>
#include <memory>
#include <netinet/in.h>
#include <vector>

namespace rocket {
void TinyPBCodec::encode(std::vector<AbstractProtocol::s_ptr> &msgs, TcpBuffer::s_ptr out_buffer) {
    for (auto & v : msgs) {
        DEBUGLOG("encode msg req id:%s, req id:%s", v->getReqId().c_str(), v->m_req_id.c_str());
        std::shared_ptr<TinyPBProtocol> msg = std::dynamic_pointer_cast<TinyPBProtocol>(v);
        int len = 0;
        const char* buf = encodeTinyPB(msg, len);
        if (buf && len != 0) {
            out_buffer->writeToBuffer(buf, len);
            free((void*)buf);
        }
    }
}

void TinyPBCodec::decode(std::vector<AbstractProtocol::s_ptr> &out_msgs, TcpBuffer::s_ptr buffer) {
    while (true) {
    std::vector<char> tmp = buffer->getBuffer();
    int start_index = buffer->readIndex();
    int end_index = -1;

    bool parse_succ = false;
    int pk_len = 0;

    int i = 0;
    for (i = start_index; i < buffer->writeIndex(); i++) {
        if (tmp[i] == TinyPBProtocol::PB_START) {
            if (i + 1 < buffer->writeIndex()) {
                pk_len = getIntFromNetByte(&tmp[i + 1]);
                DEBUGLOG("get pk_len %d", pk_len);
                int j = i + pk_len - 1;
                if (j >= buffer->writeIndex()) {
                    continue;
                }
                if (tmp[j] == TinyPBProtocol::PB_END) {
                    DEBUGLOG("find end");
                    start_index = i;
                    end_index = j;
                    parse_succ = true;
                    break;
                }
            }
        }
    }
    DEBUGLOG("i:%d, wirteindex:%d", i, buffer->writeIndex());
    if (i >= buffer->writeIndex()) {
        DEBUGLOG("decode end, read all buffer data");
        return;
    }
    if (parse_succ) {
        buffer->moveReadIndex(end_index - start_index + 1);

        std::shared_ptr<TinyPBProtocol>msg = std::make_shared<TinyPBProtocol>();
        msg->m_pk_len = pk_len;

        int req_id_len_index = start_index + sizeof(char) + sizeof(msg->m_pk_len);

        if (req_id_len_index >= end_index) {
            msg->parse_success = false;
            ERRORLOG("parser error, req_id_len_index=%d, end_index:%d", req_id_len_index, end_index);
        }
        msg->m_req_id_len = getIntFromNetByte(&tmp[req_id_len_index]);
        DEBUGLOG("parser req_id_len=%d", msg->m_req_id_len);

        int req_id_index = req_id_len_index + sizeof(msg->m_req_id_len);
        
        DEBUGLOG("req id len index:%d, req id index:%d", req_id_len_index, req_id_index);
        char req_id[100] = {0};
        std::memcpy(&req_id[0], &tmp[req_id_index], msg->m_req_id_len);
        msg->m_req_id = string(req_id);

        int method_name_len_index = req_id_index + msg->m_req_id_len;
        if (method_name_len_index >= end_index) {
            msg->parse_success = false;
            ERRORLOG("parser error, method_name_len_index=%d, end_index:%d", method_name_len_index, end_index);
            continue;
        }
        msg->m_method_name_len = getIntFromNetByte(&tmp[method_name_len_index]);

        int method_name_index = method_name_len_index + sizeof(msg->m_method_name_len);
        char method_name[100] = {0};
        std::memcpy(&method_name[0], &tmp[method_name_index], msg->m_method_name_len);
        msg->m_method_name= string(method_name);
        DEBUGLOG("parse method name:%s", msg->m_method_name.c_str());

        int err_code_index = method_name_index +msg->m_method_name_len;
        
        if (err_code_index >= end_index) {
          msg->parse_success = false;
          ERRORLOG("parse error, err_code_index[%d] >= end_index[%d]", err_code_index, end_index);
          continue;
        }
        msg->m_err_code = getIntFromNetByte(&tmp[err_code_index]);

        int error_info_len_index = err_code_index + sizeof(msg->m_err_code);
        if (error_info_len_index >= end_index) {
          msg->parse_success = false;
          ERRORLOG("parse error, error_info_len_index[%d] >= end_index[%d]", error_info_len_index, end_index);
          continue;
        }
        
        msg->m_err_info_len = getIntFromNetByte(&tmp[error_info_len_index]);
        int err_info_index = error_info_len_index + sizeof(msg->m_err_info_len);
        
        char error_info[512] = {0};
        memcpy(&error_info[0], &tmp[err_info_index],msg->m_err_info_len);
        msg->m_err_info = std::string(error_info);
        DEBUGLOG("parse error_info=%s",msg->m_err_info.c_str());

        int pb_data_len = msg->m_pk_len - msg->m_method_name_len - msg->m_req_id_len - msg->m_err_info_len - 2 - 24;
        int pd_data_index = err_info_index + msg->m_err_info_len;
        
        msg->m_pb_data = std::string(&tmp[pd_data_index], pb_data_len);
        // 这里校验和去解析
        msg->parse_success = true;
        out_msgs.push_back(msg);
    }
    }
}


const char* TinyPBCodec::encodeTinyPB(std::shared_ptr<TinyPBProtocol> msg, int &len) {
    if (msg->m_req_id.empty()) {
        msg->m_req_id = "12345678";
    }
    DEBUGLOG("req_id=%s", msg->m_req_id.c_str());

    int pk_len = 2 + 24 + msg->m_req_id.length() + msg->m_method_name.length() + msg->m_err_info.length() + msg->m_pb_data.length();
    DEBUGLOG("pk len=%d", pk_len);

    char* buf = reinterpret_cast<char*>(malloc(pk_len));
    char* tmp = buf;

    *tmp = TinyPBProtocol::PB_START;
    tmp++;

    int32_t pk_len_net = htonl(pk_len);
    memcpy(tmp, &pk_len_net, sizeof(pk_len_net));
    tmp += sizeof(pk_len_net);

    int req_id_len = msg->m_req_id.length();
    int32_t req_id_len_net = htonl(req_id_len);
    memcpy(tmp, &req_id_len_net, sizeof(req_id_len_net));
    tmp += sizeof(req_id_len_net);

    if (!msg->m_req_id.empty()) {
        memcpy(tmp, (msg->m_req_id.c_str()), req_id_len);
        tmp += req_id_len;
    }

    int method_name_len = msg->m_method_name.length();
    int32_t method_name_len_net = htonl(method_name_len);
    memcpy(tmp, &method_name_len_net, sizeof(method_name_len_net));
    tmp += sizeof(method_name_len_net);

    if (!msg->m_method_name.empty()) {
      memcpy(tmp, &(msg->m_method_name[0]), method_name_len);
      tmp += method_name_len;
    }

    int32_t err_code_net = htonl(msg->m_err_code);
    memcpy(tmp, &err_code_net, sizeof(err_code_net));
    tmp += sizeof(err_code_net);

    int err_info_len = msg->m_err_info.length();
    int32_t err_info_len_net = htonl(err_info_len);
    memcpy(tmp, &err_info_len_net, sizeof(err_info_len_net));
    tmp += sizeof(err_info_len_net);

    if (!msg->m_err_info.empty()) {
      memcpy(tmp, &(msg->m_err_info[0]), err_info_len);
      tmp += err_info_len;
    }

    if (!msg->m_pb_data.empty()) {
      memcpy(tmp, &(msg->m_pb_data[0]), msg->m_pb_data.length());
      tmp += msg->m_pb_data.length();
    }

    int32_t check_sum_net = htonl(1);
    memcpy(tmp, &check_sum_net, sizeof(check_sum_net));
    tmp += sizeof(check_sum_net);

    *tmp = TinyPBProtocol::PB_END;

    msg->m_pk_len = pk_len;
    msg->m_req_id_len = req_id_len;
    msg->m_method_name_len = method_name_len;
    msg->m_err_info_len = err_info_len;
    msg->parse_success = true;
    len = pk_len;

    DEBUGLOG("encode msg[%s] success", msg->m_req_id.c_str());
    return buf;
}

}