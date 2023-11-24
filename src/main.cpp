#include "system/io/epoll.hpp"
#include "system/io/reactor.hpp"
#include "system/network/tcp_client.hpp"
#include <chrono>
#include <sys/epoll.h>

constexpr static size_t EPollMaxEvents = 64;

using Poller = io::Epoll<EPollMaxEvents>;
using Reactor = io::Reactor<Poller>;


struct TCPClientTraits
{
  constexpr static size_t RxBufferSize = 1024 * 1024 * 1024;
};

class Client : public network::TCPClient<Client, TCPClientTraits>
{
  using TCPClient = network::TCPClient<Client, TCPClientTraits>;

public:
  explicit Client() {}

  void init(Reactor &reactor) { TCPClient::init(reactor); }

  void connect(std::string_view endpoint, size_t port)
  {
    TCPClient::connect(endpoint, port);// async connect
  }

  void login()
  {
    // send login message
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
};


int main()
{

  Poller poller{ std::chrono::seconds(1) };
  Reactor reactor{ poller };

  Client client;
  client.init(reactor);
  client.connect("challenge1.vitorian.com", 9009);

  return 0;
}
