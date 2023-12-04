#pragma once

#include <chrono>
#include <cstdint>
#include <cstring>
#include <type_traits>
namespace protocol {
using Timestamp = uint64_t;
enum class LoginCode : char { SUCCESS = 'Y', FAILURE = 'N' };


#pragma pack(1)
struct MessageHeader
{
  char msg_type;// L,E,O,G,S,or R
  uint16_t msg_len;// length in bytes including the header
  Timestamp time;// UTC in nanoseconds
  uint16_t checksum;
};

struct LoginRequest
{
  MessageHeader header;
  char user[64];
  char password[32];// default: pwd123
};


struct LoginResponse
{
  MessageHeader header;
  LoginCode code;
  char reason[32];
};

struct SubmissionRequest
{
  MessageHeader header;
  char name[64];
  char email[64];
  char repo[64];
};

struct SubmissionResponse
{
  MessageHeader header;
  char token[32];
};

struct LogoutRequest
{
  MessageHeader header;
};

struct LogoutResponse
{
  MessageHeader header;
  char reason[32];
};
#pragma pack()

inline uint16_t checksum16(const uint8_t *buf, uint32_t len)
{
  uint32_t sum = 0;
  for (uint32_t j = 0; j < len - 1; j += 2) { sum += *((uint16_t *)(&buf[j])); }

  if ((len & 1) != 0) { sum += buf[len - 1]; }

  sum = (sum >> 16) + (sum & 0xFFFF);
  sum = (sum >> 16) + (sum & 0xFFFF);

  return uint16_t(~sum);
}

template<typename MessageType> inline auto make_message()
{
  if constexpr (std::is_same_v<MessageType, LoginRequest>) {
    LoginRequest msg;
    memset(&msg, 0, sizeof(msg));

    msg.header.time = std::chrono::steady_clock::now().time_since_epoch().count();
    msg.header.msg_type = 'L';
    msg.header.msg_len = 109;

    return msg;
  } else if constexpr (std::is_same_v<MessageType, SubmissionRequest>) {

  } else {
    static_assert([]() { return false; }(), "Other Message Type is not supported.");
  }
}
}// namespace protocol
