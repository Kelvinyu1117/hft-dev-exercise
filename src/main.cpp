#include "system/io/epoll.hpp"
#include "system/io/reactor.hpp"
#include "system/network/tcp_client.hpp"
#include <chrono>
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
  if (argc < 3) { throw "Args: <remote-end-point> <port>\n"; }

  return AppOption{ std::string(argv[1]), static_cast<uint16_t>(atoi(argv[1])) };
}


struct TCPClientTraits
{
  constexpr static size_t RxBufferSize = 1024 * 1024 * 1024;
};

class Client : public network::TCPClient<Client, TCPClientTraits>
{
  using TCPClient = network::TCPClient<Client, TCPClientTraits>;

public:
  explicit Client(std::string_view endpoint, uint16_t port) : TCPClient(endpoint, port) {}

  template<typename Reactor> void start(Reactor &reactor) noexcept { TCPClient::start(reactor); }

  void login()
  {
    // send login message
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


  void on_tcp_read(const std::byte *const data, size_t cnt)
  {
    // parse the message
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

    Client client(options.endpoint, options.port);//"challenge1.vitorian.com", 9009
    client.start(reactor);

  } catch (std::exception &ex) {
    std::cout << "Exception catched: " << ex.what() << '\n';
  }


  return 0;
}
