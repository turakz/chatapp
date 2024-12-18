#ifndef _SESSION_HPP
#define _SESSION_HPP

// std
#include <boost/beast/core/error.hpp>
#include <memory>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

class session : public std::enable_shared_from_this<session> {
public:
  explicit session(
    boost::asio::io_context& io_ctx,
    std::string host,
    std::string port,
    std::string username);
  auto run() -> void;
  auto on_resolve(
    boost::beast::error_code ec,
    boost::asio::ip::tcp::resolver::results_type results) -> void;
  auto on_connect(
    boost::beast::error_code ec,
    boost::asio::ip::tcp::resolver::results_type::endpoint_type endpoint) -> void;
  auto on_handshake(boost::beast::error_code ec) -> void;
  auto identify_self() -> void;
  auto write() -> void;
  auto on_write(boost::beast::error_code ec, std::uint16_t bytes_transferred) -> void;
  auto on_read(boost::beast::error_code ec, std::uint16_t bytes_transferred) -> void;
  auto on_close(boost::beast::error_code ec) -> void;
private:
  boost::asio::ip::tcp::resolver _resolver;
  boost::beast::websocket::stream<boost::beast::tcp_stream> _webskt;
  boost::beast::flat_buffer _rsp_buffer;
  std::string _host;
  std::string _port;
  std::string _username;
};
#endif
