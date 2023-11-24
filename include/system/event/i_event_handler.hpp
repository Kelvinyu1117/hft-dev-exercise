#pragma once

#include <cstdint>

namespace event {

template<typename T> struct IEventHandler
{
  int fd() noexcept { return static_cast<T *>(this)->fd(); }

  void on_event(uint32_t event_mask) noexcept { return static_cast<T *>(this)->on_event(event_mask); }
};
}// namespace event
