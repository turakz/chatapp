#include <boost/beast/core/bind_handler.hpp>
#include <iostream>

#include <listener.hpp>
#include <websocket_session.hpp>

listener::listener(boost::asio::io_context& ioc,
  boost::asio::ip::tcp::endpoint endpoint,
  boost::shared_ptr<shared_state> const& state)
  : _ioc(ioc)
  , _acceptor(ioc)
  , _state(state)
{
  boost::beast::error_code ec;
  // open up acceptor for listening
  this->_acceptor.open(endpoint.protocol(), ec);
  if (ec)
  {
    std::cerr << "[SERVER]: error: failed to open acceptor -- " << ec.message() << std::endl;
    return;
  }

  // allow for address reuse
  this->_acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
  if (ec)
  {
    std::cerr << "[SERVER]: error: failed to set acceptor option reuse_address -- " << ec.message() << std::endl;
    return;
  }

  // bind to server address
  this->_acceptor.bind(endpoint, ec);
  if (ec)
  {
    std::cerr << "[SERVER]: error: failed to bind acceptor to endpoint -- " << ec.message() << std::endl;
    return;
  }

  // start listening for connections
  std::cout << "[SERVER]: listening...\n";
  this->_acceptor.listen(
    boost::asio::socket_base::max_listen_connections, ec);
  if (ec)
  {
    std::cerr << "[SERVER]: error: failed to start listening -- " << ec.message() << std::endl;
    return;
  }
}

auto listener::run() -> void
{
  // give the incoming connection its own strand
  this->_acceptor.async_accept(
    boost::asio::make_strand(this->_ioc),
    boost::beast::bind_front_handler(&listener::on_accept, shared_from_this()));
}

auto listener::on_accept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket) -> void
{
  if (ec)
  {
    std::cerr << "[SERVER]: error: failed to accept connectiong -- " << ec.message() << std::endl;
    return;
  }
  else
  {
    // launch new session for this connection
    boost::make_shared<websocket_session>(
      std::move(socket),
      this->_state)->run();
  }
  // the new connection gets its own strand
  this->_acceptor.async_accept(
    boost::asio::make_strand(this->_ioc),
    boost::beast::bind_front_handler(&listener::on_accept, shared_from_this()));
}
