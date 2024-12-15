// std
#include <boost/asio/strand.hpp>
#include <boost/beast/core/bind_handler.hpp>
#include <iostream>

// local
#include <chatserver.hpp>
#include <chatsession_broker.hpp>

server::server(
    boost::asio::io_context& ioc,
    boost::asio::ip::tcp::endpoint endpoint)
  : _io_ctx{ioc}
  , _acceptor{ioc}
{
  std::cout << "[SERVER]: initializing...\n";

  boost::beast::error_code ec;
  // open acceptor
  this->_acceptor.open(endpoint.protocol(), ec);
  if (ec)
  {
    std::cerr << "[SERVER]: failed to open endpoint --" << ec.message() << std::endl;
    return;
  }
  // allow for address reuse
  this->_acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
  if (ec)
  {
    std::cerr << "[SERVER]: failed to set_option: reuse_address --" << ec.message() << std::endl;
    return;
  }
  // bind to server addr
  this->_acceptor.bind(endpoint, ec);
  if (ec)
  {
    std::cerr << "[SERVER]: failed to bind endpoint --" << ec.message() << std::endl;
    return;
  }
  // listen for incoming connections
  this->_acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
  if (ec)
  {
    std::cerr << "[SERVER]: failed to establish listener --" << ec.message() << std::endl;
    return;
  }
}

auto server::run() -> void
{
  std::cout << "[SERVER]: listening...\n";
  this->accept();
}

auto server::on_accept(
  boost::beast::error_code ec,
  boost::asio::ip::tcp::socket socket) -> void
{
  if (ec)
  {
    std::cerr << "[SERVER]: failed to accept incoming connection --" << ec.message() << std::endl;
  }
  else
  {
    // create a session and run it
    std::make_shared<session_broker>(std::move(socket))->run();
  }
  // listener loop
  this->accept();
}

auto server::accept() -> void
{
  // each connection gets its own strand
  this->_acceptor.async_accept(
    boost::asio::make_strand(this->_io_ctx),
    boost::beast::bind_front_handler(&server::on_accept, shared_from_this()));
}
