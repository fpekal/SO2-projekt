#include <arpa/inet.h>
#include <atomic>
#include <cassert>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <vector>

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

  std::string receive() {
    std::string message;
    std::vector<char> buffer;
    buffer.resize(2048);

    while (true) {
      int num = ::recv(fd, buffer.data(), 2048, 0);

      if (num <= 0)
        break;

      message.append(buffer.data(), num);

      if (num < 2048)
        break;
    }

    return message;
  }

private:
  int fd;

  int port;
  std::string address;
};

void receiver_thread_func(Client &c, std::atomic<bool> &running) {
  while (running) {
    std::cout << c.receive();
  }
}

int main(int argc, char *argv[]) {
  int port = DEFAULT_PORT;
  std::string address = DEFAULT_ADDRESS;

  std::string nickname = "";
  std::cout << "Enter your nickname: ";
  std::getline(std::cin, nickname);

  std::atomic<bool> running = true;
  Client c{port, address};
  c.connect();

  std::thread receiver_thread{receiver_thread_func, std::ref(c),
                              std::ref(running)};

  while (running) {
    std::string message;
    std::getline(std::cin, message);
    c.send(std::string{"<"} + nickname + "> " + message);
  }

  receiver_thread.join();
  return 0;
}
