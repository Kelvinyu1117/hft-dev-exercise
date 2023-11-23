#include "io/epoll.hpp"
#include "io/reactor.hpp"
#include "network/tcp_client.hpp"
#include <chrono>

constexpr static size_t EPollMaxEvents = 64;


using Poller = io::Epoll<EPollMaxEvents>;
using Reactor = io::Reactor<Poller>;


struct TCPClientTraits
{
  constexpr static size_t TxBufferSize = 1024 * 1024 * 1024;
  constexpr static size_t RxBufferSize = 1024 * 1024 * 1024;
};

using TCPClient = network::TCPClient<TCPClientTraits>;

int main()
{

  Poller poller{ std::chrono::seconds(1) };
  Reactor reactor{ poller };

  TCPClient client;
  client.connect("challenge1.vitorian.com", 9009);


  return 0;
}
