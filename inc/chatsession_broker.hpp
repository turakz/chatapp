#ifndef _SESSION_BROKER_HPP
#define _SESSION_BROKER_HPP

// std
#include <unordered_map>
#include <memory>

// third party
#include <boost/beast/core.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/websocket.hpp>

#include <chatsession.hpp>

class session_broker : public std::enable_shared_from_this<session_broker> {
public:
  explicit session_broker(boost::asio::ip::tcp::socket&& socket);
  auto run() -> void;
  auto on_run() -> void;
  auto on_accept(boost::beast::error_code ec) -> void;
  auto identify_client() -> void;
  auto on_identify(boost::beast::error_code ec, std::uint16_t bytes_transferred) -> void;
  auto do_read() -> void;
  auto on_read(boost::beast::error_code ec, std::uint16_t bytes_transferred) -> void;
  auto on_write(boost::beast::error_code ec, std::uint16_t bytes_transferred) -> void;
private:
  std::string _username;
  boost::beast::websocket::stream<boost::beast::tcp_stream> _webskt;
  boost::beast::flat_buffer _buffer;
  std::unordered_map<std::string,
    boost::beast::websocket::stream<boost::beast::tcp_stream>*> _sockets;
};
#endif
