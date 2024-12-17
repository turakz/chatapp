// std
#include <boost/beast/core/bind_handler.hpp>
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

websocket_session::~websocket_session()
{
  // remove this session from list of active sessions
  this->_state->leave(this);
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

  // read a message from client
  this->_webskt.async_read(
    this->_buffer,
    boost::beast::bind_front_handler(&websocket_session::on_read, shared_from_this()));
}

auto websocket_session::on_read(boost::beast::error_code ec,
  std::uint16_t bytes_transferred) -> void
{
  if (ec)
  {
    std::cerr << "[SERVER]: error: failed to read client msg -- " << ec.message() << "\n";
    return;
  }
  // broadcast msg to all connected clients
  this->_state->send(boost::beast::buffers_to_string(this->_buffer.data()));

  // re-start read loop
  this->_webskt.async_read(
    this->_buffer,
    boost::beast::bind_front_handler(&websocket_session::on_read, shared_from_this()));
}

auto websocket_session::send(boost::shared_ptr<std::string const> const& msg) -> void
{
  // append msg to msg queue for writes
  this->_write_queue.push_back(msg);

  // are we already writing?
  if (this->_write_queue.size() > 1)
  {
    // then let that strand (or those strands) write
    return;
  }

  // since we're not currently writing, broadcast immediately
  this->_webskt.async_write(
    boost::asio::buffer(*this->_write_queue.front()),
    boost::beast::bind_front_handler(&websocket_session::on_write, shared_from_this()));
}

auto websocket_session::on_write(boost::beast::error_code ec,
  std::uint16_t bytes_transferred) -> void
{
  if (ec)
  {
    std::cerr << "[SERVER]: error: failed to write msg to client -- " << ec.message() << "\n";
    return;
  }

  // remove most recently sent message
  this->_write_queue.erase(std::begin(this->_write_queue));
  if (!this->_write_queue.empty())
  {
    // send the next msg
    this->_webskt.async_write(
      boost::asio::buffer(*this->_write_queue.front()),
      boost::beast::bind_front_handler(&websocket_session::on_write, shared_from_this()));
  }
}
