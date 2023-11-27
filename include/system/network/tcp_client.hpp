#pragma once

#include "system/event/i_event_handler.hpp"
#include "system/io/epoll.hpp"
#include "system/io/reactor.hpp"
#include <arpa/inet.h>
#include <cstdint>
#include <netdb.h>
#include <netinet/in.h>
#include <string_view>
#include <sys/socket.h>
#include <sys/types.h>

namespace network {


template<typename T, typename Traits> class TCPClient : public event::IEventHandler<TCPClient<T, Traits>>
{
  constexpr static auto RxBufferSize = Traits::RxBufferSize;

public:
  explicit TCPClient(std::string_view endpoint, uint16_t port) : endpoint_(endpoint), port_(port) {}

  void start() noexcept
  {
    fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    {
      sockaddr_in dest;
      memset(&dest, 0, sizeof(sockaddr_in));
      dest.sin_family = AF_INET;
      dest->sin_addr.s_addr = inet_addr(endpoint_.c_str());
      dest.sin_port = htons(port_);

      if (connect(fd_, (sockaddr *)&dest, sizeof(dest)) < 0) {}
    }
  }

  void send(const std::byte *const data, size_t cnt) noexcept {};

  int fd() noexcept { return fd_; }

  void on_event(uint32_t event_mask) noexcept
  {
    // read from buffer
    // static_cast<T *>(this)->on_tcp_read();
  }

private:
  std::byte rx_buffer_[RxBufferSize];
  std::string endpoint_;
  uint16_t port_;
  int fd_;
};
}// namespace network
