// std
#include <boost/asio/posix/descriptor.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/beast/core/bind_handler.hpp>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>

// third party
#include <boost/asio.hpp>
#include <boost/smart_ptr/make_shared_array.hpp>

// local
#include <chat_session.hpp>

auto main(void) -> std::int32_t
{
  auto const host = std::string{"127.0.0.1"};
  auto const port = std::string{"55422"};
  auto const msg = std::string{"default client msg"};

  boost::asio::io_context ioc{1};
  // create work for ioc
  std::cout << "[CLIENT]: spawning session...\n";
  boost::make_shared<session>(ioc, host, port)->run(msg); // async resolve
  ioc.run(); // do chatsession work
  return EXIT_SUCCESS;
}
