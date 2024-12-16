// std
#include <iostream>

// local
#include <websocket_session.hpp>

websocket_session::websocket_session(
  boost::asio::ip::tcp::socket&& socket,
  boost::shared_ptr<shared_state> const& state)
  : _webskt(std::move(socket))
  , _state(state)
{
}

auto websocket_session::run() -> void
{
  // Set suggested timeout settings for the websocket
  this->_webskt.set_option(
    boost::beast::websocket::stream_base::timeout::suggested(
        boost::beast::role_type::server));

  // Set a decorator to change the Server of the handshake
  this->_webskt.set_option(boost::beast::websocket::stream_base::decorator(
    [](boost::beast::websocket::response_type& res)
    {
      res.set(boost::beast::http::field::server,
          std::string(BOOST_BEAST_VERSION_STRING) +
              " websocket-chat-multi");
    }));

  // Accept the websocket handshake
  std::cout << "[SERVER]: accepting handshake...\n";
  this->_webskt.async_accept(
      boost::beast::bind_front_handler(
          &websocket_session::on_accept,
          shared_from_this()));
}

auto websocket_session::on_accept(boost::beast::error_code ec) -> void
{
  // Handle the error, if any
  if(ec)
  {
    std::cerr << "[SERVER]: error: failed to accept handshake...\n";
    return;
  }
  // add this session to list of active sessions
  this->_state->join(this);

  // TODO: begin read loop for websocket and walk through callstack
}
