// std
#include <cstdint>
#include <thread>

// third party
#include <boost/asio/ip/address.hpp>

// local
#include <chatserver.hpp>

auto main(void) -> std::int32_t
{
  auto const address = boost::asio::ip::make_address("127.0.0.1");
  unsigned short const port = 55422;

  int const threads_n = 1;
  boost::asio::io_context io_ctx{threads_n};

  std::make_shared<server>(io_ctx, boost::asio::ip::tcp::endpoint{address, port})->run();
  // run the io service on requested number of threads
  auto v = std::vector<std::thread> {};
  v.reserve(threads_n - 1);
  for (std::uint16_t idx = threads_n - 1; idx > 0; --idx)
  {
    v.emplace_back(
      [&io_ctx]
      {
        io_ctx.run();
      }
    );
  }
  io_ctx.run();
  return 0;
}
