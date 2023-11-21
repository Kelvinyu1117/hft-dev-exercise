#pragma once

#include <cstdint>
namespace protocol {
using Timestamp = uint64_t;

struct MessageHeader {
  char msg_type;    // L,E,O,G,S,or R
  uint16_t msg_len; // length in bytes including the header
  Timestamp time;   // UTC in nanoseconds
  uint16_t checksum;
};

struct LoginRequest {
  char user[64];
  char password[32]; // default: pwd123
};

enum class LoginCode : char { SUCCESS = 'Y', FAILURE = 'N' };
struct LoginResponse {
  LoginCode code;
  char reason[32];
};

struct SubmissionRequest {
  char name[64];
  char email[64];
  char repo[64];
};

struct SubmissionResponse {
  char token[32];
};

struct LogoutRequest {};

struct LogoutResponse {
  char reason[32];
};

} // namespace protocol
