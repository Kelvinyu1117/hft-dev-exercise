#pragma once

#include <cstdint>

namespace io {
struct IEventHandler
{
  virtual int fd() noexcept = 0;
  virtual void on_event(uint32_t event_mask) noexcept = 0;
  virtual ~IEventHandler() {}
};
}// namespace io
