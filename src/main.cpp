#include "protocol/message.hpp"
#include "system/io/epoll.hpp"
#include "system/io/reactor.hpp"
#include "system/network/tcp_client.hpp"
#include <chrono>
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <sys/epoll.h>

constexpr static size_t EPollMaxEvents = 64;

using Poller = io::Epoll<EPollMaxEvents>;
using Reactor = io::Reactor<Poller>;


struct AppOption
{
  std::string endpoint;
  uint16_t port;
};


AppOption parseOption(int argc, char *argv[])
{
  if (argc < 3) { throw std::runtime_error("Wrong Arguments: <remote-end-point> <port>\n"); }

  return AppOption{ std::string(argv[1]), static_cast<uint16_t>(atoi(argv[2])) };
}


struct TCPClientTraits
{
  constexpr static size_t RxBufferSize = 1024 * 1024;
  constexpr static size_t TxBufferSize = 1024 * 1024;
  using Reactor = Reactor;
};

class Client : public network::TCPClient<Client, TCPClientTraits>
{
  using TCPClient = network::TCPClient<Client, TCPClientTraits>;
  using Reactor = typename TCPClient::Reactor;

public:
  explicit Client(Reactor &reactor, std::string_view endpoint, uint16_t port) : TCPClient(reactor, endpoint, port) {}

  template<typename Reactor> void start(Reactor &reactor) noexcept { TCPClient::start(reactor); }

  void login()
  {
    constexpr std::string_view email = "kelvinyu1117@gmail.com\0";
    constexpr std::string_view password = "pwd123\0";

    auto msg = protocol::make_message<protocol::LoginRequest>();

    std::fill(tx_buffer_, tx_buffer_ + sizeof(msg), std::byte(0));

    memcpy(msg.user, std::data(email), std::size(email));
    memcpy(msg.password, std::data(password), std::size(password));

    msg.header.checksum = protocol::get_check_sum_from_message(msg);
    std::cout << "Login message - checksum = " << msg.header.checksum << '\n';
    memcpy(tx_buffer_, std::addressof(msg), sizeof(msg));

    std::cout << "Sending Login Request ...\n";
    send(tx_buffer_, sizeof(msg));
  }

  void logout()
  {
    // send logout message
    auto msg = protocol::make_message<protocol::LogoutRequest>();
    std::fill(tx_buffer_, tx_buffer_ + sizeof(msg), std::byte(0));

    msg.header.checksum = protocol::get_check_sum_from_message(msg);
    std::cout << "Logout - checksum = " << msg.header.checksum << '\n';
    memcpy(tx_buffer_, std::addressof(msg), sizeof(msg));

    memcpy(tx_buffer_, std::addressof(msg), sizeof(msg));

    std::cout << "Sending Logout Request ...\n";
    send(tx_buffer_, sizeof(msg));
  }

  void submit()
  {
    // send submit message
  }

  void on_tcp_connect() { login(); }

  template<typename MessageType> void handle_response(std::byte *const data, protocol::MessageHeader &header)
  {
    auto received_check_sum = header.checksum;
    MessageType msg;
    memcpy(std::addressof(msg), data, header.msg_len);
    auto check_sum = protocol::get_check_sum_from_message(msg);

    if (received_check_sum != check_sum) {
      std::cout << "checksum validation failed, received = " << received_check_sum << ", calculated = " << check_sum
                << ", drop it now...\n";
      return;
    }

    if constexpr (std::is_same_v<MessageType, protocol::LoginResponse>) {
      std::cout << "Login Response Received: reason - " << std::string_view(msg.reason) << '\n';
    } else if constexpr (std::is_same_v<MessageType, protocol::LogoutResponse>) {
      std::cout << "Logout Response Received: reason - " << std::string_view(msg.reason) << '\n';
    } else if constexpr (std::is_same_v<MessageType, protocol::SubmissionResponse>) {
      std::cout << "Submission Response Received: token - " << std::string_view(msg.token) << '\n';
    } else {
      static_assert([]() { return false; }());
    }
  }


  void on_tcp_read(std::byte *const data, size_t cnt)
  {
    protocol::MessageHeader header;
    memcpy(std::addressof(header), data, sizeof(header));
    switch (header.msg_type) {
      case 'E':
        std::cout << "Recieved Login response...\n";
        handle_response<protocol::LoginResponse>(data, header);
        break;
      case 'R':
        std::cout << "Recieved Submission response...\n";
        handle_response<protocol::SubmissionResponse>(data, header);
        break;
      case 'G':
        std::cout << "Recieved Logout response...\n";
        handle_response<protocol::LogoutResponse>(data, header);
        break;
      default:
        std::cout << "Unknown message type: " << header.msg_type << "\n";
    }
  }

  void on_tcp_disconnect() {}

  ~Client()
  {
    if (!is_logined) { logout(); }
  }

private:
  bool is_logined{ false };
};


int main(int argc, char *argv[])
{
  try {
    auto options = parseOption(argc, argv);

    Poller poller{ std::chrono::seconds(1) };
    Reactor reactor{ poller };

    Client client(reactor, options.endpoint, options.port);//"challenge1.vitorian.com", 9009
    client.start(reactor);
    reactor.run();

  } catch (std::exception &ex) {
    std::cout << "Exception catched: " << ex.what() << '\n';
  }


  return 0;
}
