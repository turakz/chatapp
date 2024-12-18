#ifndef _SHARED_STATE_HPP
#define _SHARED_STATE_HPP

// std
#include <mutex>
#include <string>
#include <unordered_set>

class websocket_session;

class shared_state {
public:
  explicit shared_state();
  auto join(websocket_session* session) -> void;
  auto leave(websocket_session* session) -> void;
  auto send(std::string msg) -> void;
private:
  std::mutex _mutex;
  std::unordered_set<websocket_session*> _sessions;
};
#endif
