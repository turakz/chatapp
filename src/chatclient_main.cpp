#include <cstdint>

#include <chatclient.hpp>

auto main(void) -> std::int32_t
{

  client chat_client {"127.0.0.1", "55422"};
  chat_client.connect();
  return 0;
}
