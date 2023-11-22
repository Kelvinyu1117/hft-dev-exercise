#include "network/tcp_client.hpp"

int main() {
  network::TCPClient client("challenge1.vitorian.com", 9009);

  return 0;
}
