// std
#include <cstdint>
#include <thread>

// third party
#include <boost/asio/signal_set.hpp>

// local
#include <listener.hpp>
#include <shared_state.hpp>

auto main(void) -> std::int32_t
{
  auto const address = boost::asio::ip::make_address("127.0.0.1");
  unsigned short const port = 55422;

  // The io_context is required for all I/O
  boost::asio::io_context ioc;

  // Create and launch a listening port
  boost::make_shared<listener>(
    ioc,
    boost::asio::ip::tcp::endpoint{address, port},
    boost::make_shared<shared_state>())->run();

  // Capture SIGINT and SIGTERM to perform a clean shutdown
  boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
  signals.async_wait(
    [&ioc](boost::system::error_code const&, int)
    {
        // Stop the io_context. This will cause run()
        // to return immediately, eventually destroying the
        // io_context and any remaining handlers in it.
        ioc.stop();
    });

  // Run the I/O service on the requested number of threads
  auto threads_n = std::uint16_t{1};
  std::vector<std::thread> work;
  work.reserve(threads_n - 1);
  for(auto i = threads_n - 1; i > 0; --i)
    work.emplace_back(
    [&ioc]
    {
        ioc.run();
    });
  ioc.run();

  // (If we get here, it means we got a SIGINT or SIGTERM)

  // Block until all the threads exit
  for(auto& t : work)
  {
    t.join();
  }
  return EXIT_SUCCESS;
}
