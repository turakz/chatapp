#ifndef _LISTENER_HPP
#define _LISTENER_HPP

// third party
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>

// local

class shared_state;

class listener : public boost::enable_shared_from_this<listener> {
public:
  listener(boost::asio::io_context& ioc,
    boost::asio::ip::tcp::endpoint endpoint,
    boost::shared_ptr<shared_state> const& state);
  auto run() -> void;
private:
  boost::asio::io_context& _ioc;
  boost::asio::ip::tcp::acceptor _acceptor;
  boost::shared_ptr<shared_state> _state;
  auto on_accept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket) -> void;
};
#endif
