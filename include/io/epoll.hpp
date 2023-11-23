#pragma once

#include "io/i_event_handler.hpp"
#include <asm-generic/errno.h>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <iostream>
#include <memory>
#include <sys/epoll.h>
#include <system_error>

namespace io {

template<size_t MaxEvents> class Epoll
{
public:
  explicit Epoll(std::chrono::microseconds timeout) : timeout_{ timeout }, epoll_fd_(epoll_create1(EPOLL_CLOEXEC))
  {
    if (epoll_fd_ == -1) { throw std::system_error(errno, std::system_category()); }
  }

  Epoll(Epoll &&) = default;


  int add_event_handler(IEventHandler *handler, uint32_t event_mask) noexcept
  {
    epoll_event event;
    memset(std::addressof(event), 0, sizeof(epoll_event));

    event.events = event_mask;
    event.data.ptr = handler;
    auto rv = ::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, handler->fd(), std::addressof(event));
    if (rv == -1) {
      std::cerr << "Epoll - failed to add event_handler for fd " << handler->fd() << " - " << std::strerror(errno)
                << '\n';
      return -errno;
    }

    return 0;
  }

  int remove_event_handler(IEventHandler *handler) noexcept
  {
    auto rv = epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, handler->fd(), nullptr);
    if (rv == -1) {
      std::cerr << "Epoll - failed to add event_handler for fd " << handler->fd() << " - " << std::strerror(errno)
                << '\n';
      return -errno;
    }

    return 0;
  }

  int poll() noexcept
  {
    int evt_cnt = epoll_wait(epoll_fd_, std::data(events_), MaxEvents, timeout_.count());

    switch (evt_cnt) {
      case 0:
        return -ETIMEDOUT;
      case -1:
        if (errno != EINTR) return -errno;
        break;
      default:
        for (size_t i = 0; i < evt_cnt; ++i) {
          auto handler = static_cast<IEventHandler *>(events_[i].data.ptr);
          handler->on_event(events_[i].events);
        }
        break;
    }

    return 0;
  }

private:
  std::chrono::microseconds timeout_;
  epoll_event events_[MaxEvents] = {};
  int epoll_fd_{ -1 };
};
}// namespace io
