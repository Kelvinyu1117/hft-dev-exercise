#pragma once

#include "system/event/i_event_handler.hpp"
#include <asm-generic/errno.h>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <iostream>
#include <memory>
#include <sys/epoll.h>
#include <sys/types.h>
#include <system_error>
#include <vector>

namespace io {

struct EpollEventData
{
  using EpollEventHandler = void (*)(uint32_t, void *);

  EpollEventHandler handler{ nullptr };
  void *user_data{ nullptr };
};

template<size_t MaxEvents> class Epoll
{
public:
  explicit Epoll(std::chrono::microseconds timeout) : timeout_{ timeout }, epoll_fd_(epoll_create1(EPOLL_CLOEXEC))
  {
    handlers.reserve(MaxEvents * 2);// just avoiding resize()
    if (epoll_fd_ == -1) { throw std::system_error(errno, std::system_category()); }
  }

  Epoll(Epoll &&) = default;


  template<typename T> int add_event_handler(event::IEventHandler<T> *handler, uint32_t event_mask) noexcept
  {
    epoll_event event;
    memset(std::addressof(event), 0, sizeof(epoll_event));

    event.events = event_mask;
    handlers.emplace_back(EpollEventData{ [](uint32_t event_masks, void *user_data) {
                                           static_cast<event::IEventHandler<T> *>(user_data)->on_event(event_masks);
                                         },
      handler });

    event.data.ptr = std::addressof(handlers.back());

    auto rv = ::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, handler->fd(), std::addressof(event));
    if (rv == -1) {
      std::cerr << "Epoll - failed to add event_handler for fd " << handler->fd() << " - " << std::strerror(errno)
                << '\n';
      return -errno;
    }

    return 0;
  }


  template<typename T> int remove_event_handler(event::IEventHandler<T> *handler) noexcept
  {
    auto rv = epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, handler->fd(), nullptr);
    if (rv == -1) {
      std::cerr << "Epoll - failed to remove event_handler for fd " << handler->fd() << " - " << std::strerror(errno)
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
          auto container = static_cast<EpollEventData *>(events_[i].data.ptr);
          container->handler(events_[i].events, container->user_data);
        }
        break;
    }

    return 0;
  }

private:
  std::chrono::microseconds timeout_;
  epoll_event events_[MaxEvents] = {};
  std::vector<EpollEventData> handlers;
  int epoll_fd_{ -1 };
};
}// namespace io
