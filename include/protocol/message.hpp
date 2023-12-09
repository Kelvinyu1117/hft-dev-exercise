#pragma once

#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
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
  MessageType msg;
  memset(&msg, 0, sizeof(msg));
  msg.header.time =
    std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  msg.header.time = 1702106441965283638;

  if constexpr (std::is_same_v<MessageType, LoginRequest>) {
    msg.header.msg_type = 'L';
    msg.header.msg_len = 109;

  } else if constexpr (std::is_same_v<MessageType, SubmissionRequest>) {
    msg.header.msg_type = 'S';
    msg.header.msg_len = 205;

  } else if constexpr (std::is_same_v<MessageType, LogoutRequest>) {
    msg.header.msg_type = 'O';
    msg.header.msg_len = 13;
  } else {
    static_assert([]() { return false; }(), "Other Message Type is not supported.");
  }

  return msg;
}

template<typename MessageType> inline uint16_t get_check_sum_from_message(const MessageType &msg)
{
  uint8_t buf[sizeof(MessageType)] = {};
  int offset = sizeof(msg.header) - sizeof(msg.header.checksum);
  // calculate checksum
  memcpy(buf, std::addressof(msg.header), offset);

  if constexpr (std::is_same_v<MessageType, LoginRequest>) {
    auto user_len = std::size(msg.user);
    auto pw_len = std::size(msg.password);

    memcpy(buf + offset, msg.user, user_len);
    offset += user_len;
    memcpy(buf + offset, msg.password, pw_len);
    offset += pw_len;

  } else if constexpr (std::is_same_v<MessageType, LoginResponse>) {
    auto reason_len = std::size(msg.reason);

    memcpy(buf + offset, std::addressof(msg.code), sizeof(msg.code));
    offset += sizeof(msg.code);

    memcpy(buf + offset, msg.reason, reason_len);
    offset += reason_len;

  } else if constexpr (std::is_same_v<MessageType, SubmissionRequest>) {
    auto name_len = std::size(msg.name);
    auto email_len = std::size(msg.email);
    auto repo_len = std::size(msg.repo);

    memcpy(buf + offset, msg.name, name_len);
    offset += name_len;
    memcpy(buf + offset, msg.email, email_len);
    offset += email_len;
    memcpy(buf + offset, msg.repo, repo_len);
    offset += repo_len;

  } else if constexpr (std::is_same_v<MessageType, SubmissionResponse>) {
    auto token_len = std::size(msg.token);

    memcpy(buf + offset, msg.token, token_len);
    offset += token_len;

  } else if constexpr (std::is_same_v<MessageType, LogoutRequest>) {
    // ...
  } else if constexpr (std::is_same_v<MessageType, LogoutResponse>) {
    auto reason_len = std::size(msg.reason);
    memcpy(buf + offset, msg.reason, reason_len);
    offset += reason_len;

  } else {
    static_assert([]() { return false; }());
  }

  return protocol::checksum16(reinterpret_cast<uint8_t *>(buf), offset);
}
}// namespace protocol
