#include <boost/beast/core/buffers_to_string.hpp>
#include <iostream>

#include <shared_state.hpp>
#include <websocket_session.hpp>

shared_state::shared_state()
{
}

auto shared_state::join(websocket_session* session) -> void
{
  std::lock_guard<std::mutex> lock(this->_mutex);
  this->_sessions.insert(session);
}

auto shared_state::leave(websocket_session* session) -> void
{
  std::lock_guard<std::mutex> lock(this->_mutex);
  this->_sessions.erase(session);
}

auto shared_state::send(std::string msg) -> void
{
  // let each client re-use the message
  auto const session_msg = boost::make_shared<std::string const>(std::move(msg));

  // create local list of all weak pointers representing sessions
  // -> can do the actual sending without holding the mutex
  std::vector<boost::weak_ptr<websocket_session>> wp_sessions;
  {
      std::lock_guard<std::mutex> lock(this->_mutex);
      wp_sessions.reserve(this->_sessions.size());
      for(auto ptr_session : this->_sessions)
          wp_sessions.emplace_back(ptr_session->weak_from_this());
  }

  // for each session in local list, try to acquire a str ptr
  // -> if we can, send a msg on that session's ptr
  for (auto const& wp_session : wp_sessions)
  {
    if (auto sp_session = wp_session.lock())
    {
      std::cout << "[SERVER]: broadcasting message -> " << *session_msg << "\n";
      sp_session->send(session_msg);
    }
  }
}
