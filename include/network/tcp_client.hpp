#pragma once

#include "io/epoll.hpp"
#include "io/reactor.hpp"
#include <string_view>

namespace network {


template<typename T, typename Traits> class TCPClient : public io::IEventHandler<TCPClient<T, Traits>>
{
  constexpr static auto RxBufferSize = Traits::RxBufferSize;

public:
  explicit TCPClient() {}

  template<typename Reactor> void init(Reactor &reactor) { reactor.add_event_handler(this, EPOLLIN | EPOLLERR); }

  void connect(std::string_view endpoint, uint16_t port) noexcept {}

  void send(const std::byte *const data, size_t cnt) noexcept {};

  int fd() noexcept { return -1; }

  void on_event(uint32_t event_mask) noexcept
  {
    // read from buffer
    // static_cast<T *>(this)->on_tcp_read();
  }

private:
  std::byte rx_buffer_[RxBufferSize];
  int fd_;
};
}// namespace network
