#ifndef _CLIENT_HPP
#define _CLIENT_HPP

// local
#include <chatsession.hpp>

class client {
public:
  explicit client(std::string host, std::string port);
  auto connect() -> void;
private:
  std::string _host;
  std::string _port;
  std::string _username;
};
#endif
