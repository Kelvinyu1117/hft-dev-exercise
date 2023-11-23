#pragma once

#include "io/epoll.hpp"
#include "io/reactor.hpp"
#include <string_view>

namespace network {


template<typename Traits> class TCPClient
{
public:
  explicit TCPClient() {}

  void connect(std::string_view endpoint, uint16_t port) {}

  void send(const std::byte *const data, size_t cnt){};


  void on_event(uint32_t event_mask)
  {
    // read from buffer
  }

private:
};
}// namespace network
