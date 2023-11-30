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
  constexpr static auto TxBufferSize = Traits::TxBufferSize;

public:
  explicit TCPClient(std::string_view endpoint, uint16_t port) : endpoint_(endpoint), port_(port) {}

  template<typename Reactor> void start(Reactor &reactor) noexcept
  {
    if (!create_socket()) { return; }
    if (!connect()) { return; }

    reactor.add_event_handler(this, EPOLLIN | EPOLLET);
  }


  void send(const std::byte *const data, size_t cnt) noexcept {};

  int fd() noexcept { return fd_; }

  void on_event(uint32_t event_mask) noexcept {}

private:
  bool create_socket() noexcept
  {
    fd_ = ::socket(AF_INET, SOCK_STREAM, 0);

    if (fd_ < 0) {
      std::cerr << "create socket failed, rc = " << fd_ << errno << '\n';
      return false;
    }

    return true;
  }


  bool connect() noexcept
  {
    // sockaddr_in dest;
    // memset(&dest, 0, sizeof(sockaddr_in));
    // dest.sin_family = AF_INET;
    // dest.sin_addr.s_addr = inet_addr(endpoint_.c_str());
    // dest.sin_port = htons(port_);

    // if (auto rc = ::connect(fd_, (sockaddr *)&dest, sizeof(dest)); rc < 0) {
    //   std::cerr << "connection failed, rc = " << rc << errno << std::endl;
    //   return false;
    // }

    addrinfo hints{};
    addrinfo *addrs;
    std::string port_s = std::to_string(port_);

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_socktype = IPPROTO_TCP;

    if (auto rc = ::getaddrinfo(endpoint_.c_str(), port_s.c_str(), &hints, &addrs); rc != 0) {
      std::cerr << "failed to resolve the Internet name to IP address rc = " << rc << errno << std::endl;
      return false;
    }

    for (auto it = addrs; it != nullptr; it = it->ai_next) {
      // std::cout << it->ai_family << " " << it->ai_family << " " << it->ai_protocol << '\n';
      char addr_buffer[INET6_ADDRSTRLEN];
      auto pt = (sockaddr_in *)it->ai_addr;
      if (inet_ntop(it->ai_family, &pt->sin_addr, addr_buffer, sizeof(addr_buffer))) std::cout << addr_buffer << '\n';
    }


    return true;
  }


private:
  std::byte rx_buffer_[RxBufferSize];
  std::byte tx_buffer[TxBufferSize];
  std::string endpoint_;
  uint16_t port_;
  int fd_;
};
}// namespace network
