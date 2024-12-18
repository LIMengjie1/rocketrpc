#include "codec/abstract_protocol.h"
#include "codec/tinypb_protocol.h"
#include "net_addr.h"
#include "rocket/common/log.h"
#include "rocket/common/config.h"
#include "rocket/net/fd_event.h"
#include "rocket/net/eventloop.h"
#include "rocket/net/timer_event.h"
#include "rocket/net/io_thread.h"
#include "rocket/net/io_thread_group.h"
#include "rocket/net/tcp/tcp_server.h"
#include "tcp_client.h"
#include "codec/string_codec.h"
#include <functional>
#include <iostream>
#include <memory>
#include <sys/socket.h>

#include "order.pb.h"

void test_connect() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);

  if (fd < 0) {
    ERRORLOG("invalid fd %d", fd);
    exit(0);
  }

  sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(12345);
  inet_aton("127.0.0.1", &server_addr.sin_addr);

  int rt = connect(fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));

  DEBUGLOG("connect success");

  std::string msg = "hello rocket!";
  
  rt = write(fd, msg.c_str(), msg.length());

  DEBUGLOG("success write %d bytes, [%s]", rt, msg.c_str());

  char buf[100];
  rt = read(fd, buf, 100);
  DEBUGLOG("success read %d bytes, [%s]", rt, std::string(buf).c_str());
}

void test_tcp_server() {
    rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 12345);
    DEBUGLOG("get addr%s", addr->toString().c_str());

    rocket::TcpServer tcp_server(addr);
    tcp_server.start();
}

void test_tcp_client() {
  rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 12345);
  rocket::TcpClient client(addr);

  client.connect([&](){
    DEBUGLOG("connect to %s succ", addr->toString().c_str());
    std::shared_ptr<rocket::TinyPBProtocol> msg = make_shared<rocket::TinyPBProtocol>();
    msg->setReqId("12345");
    
    makeOrderRequest request;
    request.set_price(100);
    request.set_goods("apple");

    if (!request.SerializeToString(&(msg->m_pb_data))) {
        ERRORLOG("seilize error");
        return;
    }
    msg->m_method_name = "Order.makeOrder";

    DEBUGLOG("ERROR: client writeMessage");
    client.writeMessage(msg, [request](rocket::AbstractProtocol::s_ptr done) {
      DEBUGLOG("send msg succ request[%s]", request.ShortDebugString().c_str());
    });
    DEBUGLOG("ERROR: client readMessage");
    client.readMessage("12345", [](rocket::AbstractProtocol::s_ptr done) {
     std::shared_ptr<rocket::TinyPBProtocol> msg = std::dynamic_pointer_cast<rocket::TinyPBProtocol>(done);
     DEBUGLOG("get response:%s, info:%s", msg->getReqId().c_str(), msg->m_pb_data.c_str());
     makeOrderResponse response;
     if (!response.ParseFromString(msg->m_pb_data)) {
        ERRORLOG("deserialize error");
        return;
     }
     DEBUGLOG("get reponse succ, reponse:%s", response.ShortDebugString().c_str());
    });
  });
}

int main() {
    rocket::Config::SetGlobalConfig("../conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();
    test_tcp_client();
}