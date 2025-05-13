#include <arpa/inet.h>
#include <cassert>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>

int DEFAULT_PORT = 2137;
std::string DEFAULT_ADDRESS = "127.0.0.1";

class Client {
public:
  Client(int port, const std::string &address)
      : port{port}, address{address}, fd{0} {}

  void connect() {

    fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(fd > 0);

    sockaddr_in sin;
    sin.sin_port = htons(port);
    sin.sin_family = AF_INET;
    inet_pton(AF_INET, address.c_str(), &sin.sin_addr);

    {
      int ret = ::connect(fd, (sockaddr *)&sin, sizeof(sockaddr_in));
      assert(ret == 0);
    }
  }

  void send(const std::string &message) {
    ::send(fd, message.c_str(), message.length(), 0);
  }

private:
  int fd;

  int port;
  std::string address;
};

int main(int argc, char *argv[]) {
  Client c{2137, "127.0.0.1"};
  c.connect();
  c.send("elo\n");
  return 0;
}
