#ifndef _WEBSOCKET_SESSION_HPP
#define _WEBSOCKET_SESSION_HPP

// std
#include <vector>

// third party
#include <boost/beast.hpp>

// local
#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include <shared_state.hpp>

class shared_state;

class websocket_session : public boost::enable_shared_from_this<websocket_session> {
public:
  websocket_session(
    boost::asio::ip::tcp::socket&& socket,
    boost::shared_ptr<shared_state>const& state);
  ~websocket_session();
  auto run() -> void;
private:
  boost::beast::flat_buffer _buffer;
  boost::beast::websocket::stream<boost::beast::tcp_stream> _webskt;
  boost::shared_ptr<shared_state> _state;
  std::vector<boost::shared_ptr<std::string const>> queue_;

  auto on_accept(boost::beast::error_code ec) -> void;
  auto on_read(boost::beast::error_code ec, std::uint16_t bytes_transferred) -> void;
  auto on_write(boost::beast::error_code ec, std::uint16_t bytes_transferred) -> void;
};
#endif
