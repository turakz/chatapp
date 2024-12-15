// std
#include <boost/beast/core/bind_handler.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/core/stream_traits.hpp>
#include <boost/beast/version.hpp>
#include <iostream>
#include <chrono>

// third party
#include <boost/asio/strand.hpp>

// local
#include <chatsession.hpp>

session::session(
  boost::asio::io_context& io_ctx,
  std::string host,
  std::string port,
  std::string username)
  : _resolver{io_ctx}
  , _webskt{boost::asio::make_strand(io_ctx)}
  , _host{host}
  , _port{port}
  , _username{username}
{
  std::cout << "session initialized...\n";
}

auto session::run() -> void
{
  std::cout << "resolving connection to server address...\n";
  // look up domain name
  this->_resolver.async_resolve(
    this->_host,
    this->_port,
    boost::beast::bind_front_handler(&session::on_resolve, shared_from_this()));
}

auto session::on_resolve(
  boost::beast::error_code ec,
  boost::asio::ip::tcp::resolver::results_type results) -> void
{
  if (ec)
  {
    std::cerr << "error: client failed to resolve server address -- " << ec.message() << std::endl;
    return;
  }

  std::cout << "connecting...\n";

  // set timeout for operation
  boost::beast::get_lowest_layer(this->_webskt).expires_after(std::chrono::seconds(30));

  // make connection to ip address we get from lookup
  boost::beast::get_lowest_layer(this->_webskt).async_connect(
    results,
    boost::beast::bind_front_handler(&session::on_connect, shared_from_this()));
}

auto session::on_connect(
  boost::beast::error_code ec,
  boost::asio::ip::tcp::resolver::results_type::endpoint_type endpoint) -> void
{
  if (ec)
  {
    std::cerr << "error: client failed to connect to server address -- " << ec.message() << std::endl;
    return;
  }

  std::cout << "handshaking...\n";

  // turn off timeout on stream bc websocket has its own timeout system
  boost::beast::get_lowest_layer(this->_webskt).expires_never();

  // set suggested timeout settings for websocket
  this->_webskt.set_option(
    boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::client));

  // set decorator to change the user-agent of the handshake
  this->_webskt.set_option(boost::beast::websocket::stream_base::decorator(
    [](boost::beast::websocket::request_type& req)
    {
      req.set(boost::beast::http::field::user_agent,
        std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client-async");
    }));

  // update host string to provide value of the host http header during handshake
  this->_host += ':' + std::to_string(endpoint.port());

  // perform websocket handshake
  this->_webskt.async_handshake(this->_host, "/",
    boost::beast::bind_front_handler(&session::on_handshake, shared_from_this()));
}

auto session::on_handshake(boost::beast::error_code ec) -> void
{
  if (ec)
  {
    std::cerr << "error: client failed to handshake connection -- " << ec.message() << std::endl;
    return;
  }
  std::cout << "client connected...\n";
  //this->write();
  this->identify_self();
}

auto session::identify_self() -> void
{
  // clear buffer and prepare for new response
  this->_rsp_buffer.consume(this->_rsp_buffer.size());
  this->_webskt.async_write(
    boost::asio::buffer(this->_username),
    boost::beast::bind_front_handler(&session::on_write, shared_from_this()));
}

auto session::write() -> void
{
  // clear buffer and prepare for new response
  this->_rsp_buffer.consume(this->_rsp_buffer.size());
  // send a msg
  std::cout << "> ";
  auto msg = std::string {};
  std::getline(std::cin, msg);

  this->_webskt.async_write(
    boost::asio::buffer(msg),
    boost::beast::bind_front_handler(&session::on_write, shared_from_this()));
}

auto session::on_write(boost::beast::error_code ec, std::uint16_t bytes_transferred) -> void
{
  if (ec)
  {
    std::cerr << "error: client failed to write msg to socket -- " << ec.message() << std::endl;
    return;
  }

  // read response from server
  this->_webskt.async_read(
    this->_rsp_buffer,
    boost::beast::bind_front_handler(&session::on_read, shared_from_this()));
}

auto session::on_read(boost::beast::error_code ec, std::uint16_t bytes_transferred) -> void
{
  if (ec == boost::asio::error::eof or ec == boost::beast::websocket::error::closed)
  {
    std::cout << "server closed connection...\n";
    this->_webskt.async_close(boost::beast::websocket::close_code::normal,
      boost::beast::bind_front_handler(&session::on_close, shared_from_this()));
  }
  if (ec)
  {
    std::cerr << "error: client failed to read msg from socket -- " << ec.message() << std::endl;
    return;
  }
  std::cout << boost::beast::buffers_to_string(this->_rsp_buffer.data()) << "\n";
  this->write();
}

auto session::on_close(boost::beast::error_code ec) -> void
{
  if (ec)
  {
    std::cerr << "error: client failed to close socket -- " << ec.message() << std::endl;
    return;
  }
  std::cout << "closing connection..." << std::endl;
}
