#pragma once
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <memory>

namespace rocket {

class NetAddr {
 public:
  typedef std::shared_ptr<NetAddr> s_ptr;

  virtual sockaddr* getSockAddr() = 0;

  virtual socklen_t getSockLen() = 0;

  virtual int getFamily() = 0;

  virtual std::string toString() = 0;

  virtual bool checkValid() = 0;

};


class IPNetAddr : public NetAddr {

 public:
  static bool CheckValid(const std::string& addr);

 public:
  
  IPNetAddr(const std::string& ip, uint16_t port);
  
  IPNetAddr(const std::string& addr);

  IPNetAddr(sockaddr_in addr);

  sockaddr* getSockAddr() override;

  socklen_t getSockLen() override;

  int getFamily() override;

  std::string toString() override;

  bool checkValid() override;
 
 private:
  std::string m_ip;
  uint16_t m_port {0};
  sockaddr_in m_addr;

};

}

