// std
#include <iostream>

// local
#include <chatclient.hpp>
#include <chatsession.hpp>
#include <chatsession_broker.hpp>

client::client(std::string host, std::string port)
  : _host{host}
  , _port{port}
  , _username{""}
{
  std::cout << "client initialized...\n";
}

auto client::connect() -> void
{
  std::cout << "client connecting...\n";
  std::cout << "username: ";
  std::getline(std::cin, this->_username);
  // io ctx required for all io
  boost::asio::io_context io_ctx;
  std::make_shared<session>(io_ctx, this->_host, this->_port, this->_username)->run();

  // run the io service -- this call will return when the socket is closed
  io_ctx.run();
}
