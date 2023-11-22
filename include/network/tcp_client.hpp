#pragma once

#include "io/epoll.hpp"
#include "io/reactor.hpp"
#include <string_view>

namespace network {

class TCPClient {
public:
  explicit TCPClient(std::string_view endpoint, uint64_t port) {}

  void on_event() {}

private:
  io::Reactor<io::Epoll> reactor_;
  char tx_buffer_[1024 * 1024 * 1024];
  char rx_buffer_[1024 * 1024 * 1024];
};
} // namespace network
