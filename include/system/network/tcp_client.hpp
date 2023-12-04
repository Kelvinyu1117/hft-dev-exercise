#pragma once

#include "system/event/i_event_handler.hpp"
#include "system/io/epoll.hpp"
#include "system/io/reactor.hpp"
#include <arpa/inet.h>
#include <cstdint>
#include <cstring>
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
    reactor.add_event_handler(this, EPOLLIN | EPOLLET);
    if (!create_socket()) { return; }
    if (!connect()) {
      return;
    } else {
      on_tcp_connect();
    }
  }

  void send(const std::byte *const data, size_t n) noexcept
  {
    memcpy(rx_buffer_, data, n);
    int total_sent = 0;
    while (total_sent < n) {
      auto n_bytes = ::send(fd_, tx_buffer_ + total_sent, n - total_sent, 0);
      if (n_bytes == -1) {
        std::cout << "[send] - Failed - total_byte_sent: " << total_sent << ", bytes remaining: " << n << '\n';
      }

      total_sent += n_bytes;
    }
  }

  int fd() noexcept { return fd_; }

  void on_event(uint32_t event_mask) noexcept
  {
    auto n_bytes = ::recv(fd_, rx_buffer_, sizeof(rx_buffer_), 0);
    if (n_bytes > 0)
      on_tcp_read(rx_buffer_, n_bytes);
    else if (n_bytes == -1) {
      std::cout << "[recv] - Failed \n";
    }
  }

  void on_tcp_connect() { static_cast<T *>(this)->on_tcp_connect(); }

  void on_tcp_read(const std::byte *const data, size_t n) { static_cast<T *>(this)->on_tcp_read(data, n); }

  void on_tcp_disconnect() {}

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
    addrinfo hints{};
    addrinfo *addrs;
    std::string port_s = std::to_string(port_);

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    std::cout << "endpoint: " << endpoint_ << " port: " << port_s << '\n';
    if (auto rc = ::getaddrinfo(endpoint_.c_str(), port_s.c_str(), &hints, &addrs); rc != 0) {
      std::cerr << "failed to resolve the Internet name to IP address rc = " << rc << errno << std::endl;
      return false;
    }

    bool done = false;
    for (auto it = addrs; it != nullptr && !done; it = it->ai_next) {
      // std::cout << it->ai_family << " " << it->ai_family << " " << it->ai_protocol << '\n';
      char addr_buffer[INET6_ADDRSTRLEN];
      if (it->ai_family == AF_INET) {
        auto pt = (sockaddr_in *)it->ai_addr;
        if (inet_ntop(it->ai_family, &pt->sin_addr, addr_buffer, sizeof(addr_buffer))) {
          std::cout << "Found IP: " << addr_buffer << ", trying to connect...." << '\n';
          sockaddr_in dest;
          memset(&dest, 0, sizeof(sockaddr_in));
          dest.sin_family = AF_INET;
          memcpy(&dest.sin_addr, &pt->sin_addr, sizeof(dest.sin_addr));

          dest.sin_port = htons(port_);

          if (auto rc = ::connect(fd_, (sockaddr *)&dest, sizeof(dest)); rc < 0) {
            std::cerr << "connection failed, rc = " << rc << errno << std::endl;
          } else {
            std::cout << "The Connection is established.\n";
            done = true;
          }
        }
      }
    }


    return done;
  }


private:
  std::byte rx_buffer_[RxBufferSize];
  std::byte tx_buffer_[TxBufferSize];
  std::string endpoint_;
  uint16_t port_;
  int fd_;
};
}// namespace network
