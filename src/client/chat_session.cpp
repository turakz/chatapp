// std
#include <iostream>

// local
#include <chat_session.hpp>

session::session(boost::asio::io_context& io_ctx, std::string host, std::string port)
  : _resolver{boost::asio::make_strand(io_ctx)}
  , _webskt{boost::asio::make_strand(io_ctx)}
  , _host{host}
  , _port{port}
{
}

auto session::run(std::string msg) -> void
{
  this->_msg = std::move(msg);
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
    std::cerr << "[CLIENT]: failed to resolve "
      << "host: " << this->_host
      << ", port: " << this->_port
      << " -- " << ec.message() << "\n";
    return;
  }
  // set timeout
  boost::beast::get_lowest_layer(this->_webskt).expires_after(std::chrono::seconds(3));
  // make connection on ip address we got from lookup
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
    std::cerr << "[CLIENT]: failed to connect -- " << ec.message() << "\n";
  }

  // turn off timeout to tcp_stream, bc websocket stream has its own timeout system
  boost::beast::get_lowest_layer(this->_webskt).expires_never();
  // set suggested timeout for websocket
  this->_webskt.set_option(
    boost::beast::websocket::stream_base::timeout::suggested(
      boost::beast::role_type::client));
  // set a decorator to change the user-agent of the handshake
  this->_webskt.set_option(boost::beast::websocket::stream_base::decorator(
    [](boost::beast::websocket::request_type& req)
    {
      req.set(boost::beast::http::field::user_agent,
          std::string(BOOST_BEAST_VERSION_STRING) +
          " websocket-client-async");
    }));

  // update host string. this will provide the value of the host http header during websocket
  // handshake
  auto updated_host = (this->_host += ':' + std::to_string(endpoint.port()));
  // perform handshake
  std::cout << "[CLIENT]: client connected, performing handshake...\n";
  this->_webskt.async_handshake(updated_host, "/",
    boost::beast::bind_front_handler(&session::on_handshake, shared_from_this()));
}

auto session::on_handshake(boost::beast::error_code ec) -> void
{
  if (ec)
  {
    std::cerr << "[CLIENT]: error: failed to handshake with client -- " << ec.message() << "\n";
    return;
  }
  std::cout << "[CLIENT]: handshake successful... welcome, user!\n";
  // send msg
  this->_webskt.async_write(
    boost::asio::buffer(this->_msg),
    boost::beast::bind_front_handler(&session::on_write, shared_from_this()));
}

auto session::on_write(boost::beast::error_code ec, std::uint16_t bytes_transferred) -> void
{
  if (ec)
  {
    std::cerr << "[CLIENT]: failed to write to server -- " << ec.message() << "\n";
    return;
  }
  // successfully sent msg to server, read response back
  this->_buffer.consume(this->_buffer.size());
  this->_webskt.async_read(
    this->_buffer,
    boost::beast::bind_front_handler(&session::on_read, shared_from_this()));
}

auto session::on_read(boost::beast::error_code ec, std::uint16_t bytes_transferred) -> void
{
  if (ec)
  {
    std::cerr << "[CLIENT]: failed to read server response -- " << ec.message() << "\n";
    return;
  }

  // for now we'll just close socket
  std::cout << "[CLIENT]: recieved msg from server -> " << boost::beast::buffers_to_string(this->_buffer.data());
  this->_webskt.async_close(boost::beast::websocket::close_code::normal,
    boost::beast::bind_front_handler(&session::on_close, shared_from_this()));
}

auto session::on_close(boost::beast::error_code ec) -> void
{
  if (ec)
  {
    std::cerr << "[CLIENT]: failed to close websocket -- " << ec.message() << "\n";
    return;
  }
  // connection closed gracefully
  std::cout << "[CLIENT]: closing connection... goodbye, user!" << std::endl;
}
