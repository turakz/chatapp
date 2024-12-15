// std
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/core/ignore_unused.hpp>
#include <iostream>

// third party
#include <boost/asio/dispatch.hpp>
#include <boost/beast/core/bind_handler.hpp>

// local
#include <chatsession_broker.hpp>

session_broker::session_broker(boost::asio::ip::tcp::socket&& socket)
  : _webskt{std::move(socket)}
{
}

auto session_broker::run() -> void
{
  // need to be executing within a strand to perform async operations on io objects
  // in the session
  boost::asio::dispatch(this->_webskt.get_executor(),
      boost::beast::bind_front_handler(&session_broker::on_run, shared_from_this()));
}

auto session_broker::on_run() -> void
{
  // set suggested timeout settings for websocket
  this->_webskt.set_option(
      boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::server));

  // set decorator to change server of handshake
  this->_webskt.set_option(boost::beast::websocket::stream_base::decorator(
    [](boost::beast::websocket::response_type& res)
    {
      res.set(boost::beast::http::field::server,
          std::string(BOOST_BEAST_VERSION_STRING) + " websocket-server-async");
    }));

  // accept the websocket handshake
  this->_webskt.async_accept(
      boost::beast::bind_front_handler(&session_broker::on_accept, shared_from_this()));
}

auto session_broker::on_accept(boost::beast::error_code ec) -> void
{
  if (ec)
  {
    std::cerr << "[SERVER]: failed to accept websocket handshake for session -- " << ec.message() << std::endl;
  }
  //this->do_read();
  this->identify_client();
}

auto session_broker::identify_client() -> void
{
  std::cout << "[SERVER]: identifying client...\n";
  this->_webskt.async_read(
    this->_buffer,
    boost::beast::bind_front_handler(&session_broker::on_identify, shared_from_this()));
}

auto session_broker::on_identify(boost::beast::error_code ec, std::uint16_t bytes_transferred) -> void
{
  this->_username = boost::beast::buffers_to_string(this->_buffer.data());
  auto response = std::string{"[SERVER]: welcome, "} + this->_username + "!";
  this->_webskt.async_write(
    boost::asio::buffer(response),
    boost::beast::bind_front_handler(&session_broker::on_write, shared_from_this()));
}

auto session_broker::do_read() -> void
{
  std::cout << "[SERVER]: reading msgs...\n";

  this->_webskt.async_read(
    this->_buffer,
    boost::beast::bind_front_handler(&session_broker::on_read, shared_from_this()));
}

auto session_broker::on_read(boost::beast::error_code ec, std::uint16_t bytes_transferred) -> void
{
  boost::ignore_unused(bytes_transferred);

  // handle session being closed
  if (ec == boost::asio::error::eof or ec == boost::beast::websocket::error::closed)
  {
    std::cerr << "[SERVER]: client disconnected...\n";
    return;
  }
  if (ec)
  {
    std::cerr << "[SERVER]: failed read from client -- " << ec.message() << std::endl;
    return;
  }

  this->_webskt.text(this->_webskt.got_text());
  auto response = std::string{"[SERVER]: received msg -> "} + boost::beast::buffers_to_string(this->_buffer.data());
  this->_webskt.async_write(
    boost::asio::buffer(response),
    boost::beast::bind_front_handler(&session_broker::on_write, shared_from_this()));
}

auto session_broker::on_write(boost::beast::error_code ec, std::uint16_t bytes_transferred) -> void
{
  boost::ignore_unused(bytes_transferred);
  if (ec)
  {
    std::cerr << "[SERVER]: failed to write to client -- " << ec.message() << std::endl;
  }

  // clear buffer
  this->_buffer.consume(this->_buffer.size());

  // listen for another client msg
  this->do_read();
}
