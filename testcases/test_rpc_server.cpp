#include <assert.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <memory>
#include <unistd.h>
#include <google/protobuf/service.h>
#include "rocket/common/log.h"
#include "rocket/common/config.h"
#include "rocket/common/log.h"
#include "rocket/net/tcp/tcp_client.h"
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/codec/abstract_protocol.h"
#include "rocket/net/codec/tinypb_codec.h"
#include "rocket/net/codec/tinypb_protocol.h"
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/tcp/tcp_server.h"
#include "rocket/net/rpc/rpc_dispatcher.h"
#include "rocket/net/rpc/rpc_controller.h"

#include "order.pb.h"

class OrderImp : public Order {
public:
    void makeOrder(google::protobuf::RpcController* controller,
                       const ::makeOrderRequest* request,
                       ::makeOrderResponse* response,
                       ::google::protobuf::Closure* done) {
        if (request->price() < 10) {
            response->set_ret_code(-1);
            response->set_res_info("short balance");
            return;
        }
        //response->set_order_id("today");
        response->set_res_info("today");
    }
};


void test_tcp_server() {
    rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 12345);
    DEBUGLOG("get addr%s", addr->toString().c_str());

    rocket::TcpServer tcp_server(addr);
    tcp_server.start();
}

int main() {
    rocket::Config::SetGlobalConfig("../conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();
    rocket::RpcDispatcher::GetRpcDispatcher()->registerService(std::make_shared<OrderImp>());
    test_tcp_server();
    return 0;
}

