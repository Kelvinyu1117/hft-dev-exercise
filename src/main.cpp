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

    uint8_t buf[sizeof(protocol::LoginRequest)] = {};

    std::fill(tx_buffer_, tx_buffer_ + sizeof(msg), std::byte(0));

    memcpy(msg.user, std::data(email), std::size(email));
    memcpy(msg.password, std::data(password), std::size(password));

    msg.header.checksum = protocol::get_check_sum_from_message(msg);
    std::cout << "checksum = " << msg.header.checksum << '\n';
    memcpy(tx_buffer_, std::addressof(msg), sizeof(msg));

    std::cout << "Sending Login Request ...\n";
    send(tx_buffer_, sizeof(msg));
  }

  void logout()
  {
    // send logout message
  }

  void submit()
  {
    // send submit message
  }

  void on_tcp_connect() { login(); }

  template<typename MessageType> void handle_response(std::byte *const data, protocol::MessageHeader &header)
  {
    auto received_check_sum = header.checksum;
    header.checksum = 0;
    MessageType msg;
    memcpy(std::addressof(msg), data, header.msg_len);

    if constexpr (std::is_same_v<MessageType, protocol::LoginResponse>) {
      std::cout << "Login Reponse Received: reason - " << std::string_view(msg.reason) << '\n';

      // if (received_check_sum != protocol::checksum16(reinterpret_cast<uint8_t *>(data), sizeof(MessageType))) {
      //   logout();
      // } else {
      // }
    } else if constexpr (std::is_same_v<MessageType, protocol::LogoutResponse>) {
      std::cout << "Logout Reponse Received: reason - " << std::string_view(msg.reason) << '\n';
    }
  }


  void on_tcp_read(std::byte *const data, size_t cnt)
  {
    protocol::MessageHeader header;
    memcpy(std::addressof(header), data, sizeof(header));
    switch (header.msg_type) {
      case 'E':
        handle_response<protocol::LoginResponse>(data, header);
        break;
      case 'R':
        std::cout << "R\n";
        handle_response<protocol::SubmissionResponse>(data, header);
        break;
      case 'G':
        std::cout << "G\n";
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
