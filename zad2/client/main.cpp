#include <arpa/inet.h>
#include <atomic>
#include <cassert>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

int DEFAULT_PORT = 2137;
std::string DEFAULT_ADDRESS = "127.0.0.1";

/** @class Client
 * @brief Chat client
 *
 * Connects to the server. Allows to send and receive messages.
 * Messages are just plaintext ASCII-encoded strings containing both nickname
 * and a "message".
 */
class Client {
public:
  /**
   * @brief Constructs a Client object.
   *
   * @param port Port to which the client will connect.
   * @param address IP address of the server.
   */
  Client(int port, const std::string &address)
      : port{port}, address{address}, fd{0} {}
  ~Client() { close(fd); }

  /** @fn void connect()
   * @brief Connect to the server.
   *
   * Creates a socket `fd` and connects it to the server specified in the
   * constructor.
   */
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

  /** @fn void send(const std::string &message)
   * @brief Send a message
   *
   * Sends a message.
   * The message should already contain the user's nickname.
   *
   * @param message Message to send
   */
  void send(const std::string &message) {
    ::send(fd, message.c_str(), message.length(), 0);
  }

  /** @fn std::string receive()
   * @brief Receive a message
   *
   * Blocks until new message appears from the server.
   * Then it receives it and returns as a std::string.
   */
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

static void receiver_thread_func(Client &c, std::atomic<bool> &running) {
  while (running) {
    std::cout << c.receive();
  }
}

int main(int argc, char *argv[]) {
  int port = DEFAULT_PORT;
  std::string address = DEFAULT_ADDRESS;

  if (argc >= 2) {
    std::string arg1 = argv[1];
    if (arg1 == "--help" || arg1 == "-h") {
      std::cout << "Usage:\n" << argv[0] << " [ip-address [port]]\n";
      return 0;
    }
    address = argv[1];
  }
  if (argc >= 3) {
    try {
      port = std::stoi(argv[2]);
    } catch (...) {
      std::cerr << "Wrong arguments.\n";
      return 1;
    }
  }
  if (argc > 3) {
    std::cerr << "Too many arguments.\n";
    return 1;
  }

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
    c.send(std::string{"<\e[0;36m"} + nickname + "\e[0m> " + message);
  }

  receiver_thread.join();
  return 0;
}
