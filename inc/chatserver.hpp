#ifndef _CHATSERVER_HPP
#define _CHATSERVER_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/core/error.hpp>

class server : public std::enable_shared_from_this<server> {
public:
  server(
    boost::asio::io_context& io_ctx,
    boost::asio::ip::tcp::endpoint endpoint);
  auto run() -> void;
private:
  boost::asio::io_context& _io_ctx;
  boost::asio::ip::tcp::acceptor _acceptor;
  auto on_accept(
    boost::beast::error_code ec,
    boost::asio::ip::tcp::socket socket) -> void;
  auto accept() -> void;
};
#endif
