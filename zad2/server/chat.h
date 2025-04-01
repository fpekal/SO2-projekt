#pragma once
#include <algorithm>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>

#include "server.h"

/** @class Client
 * @brief Chat client
 *
 * Class allowing for sending and receiving messages from the clients.
 * Closes file descriptor when the destructor is called.
 *
 * Messages are just ASCII-encoded strings containing both nickname and a
 * "message" from this client. So you can send many messages at once by just
 * concatenating them and separating with new line character (`\n`)
 */
class Client {
public:
  /** @fn Client::Client()
   * @brief Construct a Client object connected to the `fd` socket.
   *
   * @param fd Socket where the client is connected.
   */
  Client(int fd) : fd{fd} {}
  ~Client() { close(fd); }

  /** @fn void Client::send(const std::string &message)
   * @brief Sends a message(s) to the client.
   *
   * It takes a message and sends it by using `write()`.
   * Additionaly it locks a mutex so only one thread can send messages to the
   * client at a time. It is needed because there was a possibility of sending
   * malformed messages.
   *
   * @param message The string data to be transmitted to the client.
   * @throws std::runtime_error if an error occurs during `write()`. It is
   * assumed that the client has disconnected from the server.
   */
  void send(const std::string &message) {
    std::unique_lock<std::mutex> lock{send_mutex};

    int ret = write(fd, message.c_str(), message.length());
    if (ret == -1) {
      throw std::runtime_error{"Client disconnected"};
    }
  }

  /** @fn std::string Client::receive()
   * @brief Receives a message from the client.
   *
   * Reads up to 1023 bytes from the socket `fd` into the member
   * buffer. The received data in the buffer is null-terminated.
   * Then it is added to the return message.
   * Everything repeats until the whole message is received and then the message
   * is returned.
   *
   * @return std::string containing the message read from the socket.
   * @throws std::runtime_error if an error occurs during the read system call
   * (read returns -1). It is assumed that the client disconnected from the
   * server.
   * @warning If client sends exactly `N*1024+1023` characters then this
   * function will block until the next message and only then this message will
   * be received correctly. I don't want to think about how to fix this; it's
   * not the subject of this project.
   */
  std::string receive() {
    std::string message;

    while (true) {
      char buf[1024];
      int len = read(fd, buf, 1023);
      if (len <= 0)
        throw std::runtime_error{"Client disconnected"};

      buf[len] = 0;
      message += buf;

      if (len < 1023)
        break;
    }

    return message;
  }

private:
  /** @var std::mutex Client::send_mutex
   * @brief Locks an access to sending messages to the client.
   */
  std::mutex send_mutex;
  /** @var int Client::fd
   * @brief File descriptor of a socket where client is connected.
   */
  int fd;
};

class Chat {
public:
  Chat()
      : server{std::bind(&Chat::client_handler, this, std::placeholders::_1)} {}

  void run() {
    while (true) {
      server.accept_connection();
    }
  }

private:
  void client_handler(int fd) {
    auto client = add_client(fd);

    auto history = get_history();
    client->send(history);

    while (true) {
      try {
        auto new_message = client->receive();

        add_history(new_message);

        send_to_all(new_message);
      } catch (std::runtime_error &e) {
        break;
      }
    }

    remove_client(client);
  }

  Server server;

  std::string get_history() {
    std::unique_lock<std::mutex> lock{history_mutex};

    return history;
  }

  void add_history(const std::string &message) {
    std::unique_lock<std::mutex> lock{history_mutex};

    history += message.substr(0, message.length() - 2);
    history += "\n";
  }

  std::shared_ptr<Client> add_client(int fd) {
    auto client = std::make_shared<Client>(fd);

    std::unique_lock<std::mutex> lock{clients_mutex};
    clients.push_back(client);
    return client;
  }

  void remove_client(std::shared_ptr<Client> client) {
    std::unique_lock<std::mutex> lock{clients_mutex};
    auto iter = std::find(clients.begin(), clients.end(), client);
    clients.erase(iter);
  }

  void send_to_all(const std::string &message) {
    std::unique_lock<std::mutex> lock{clients_mutex};

    for (auto client : clients) {
      client->send(message);
    }
  }

  std::mutex history_mutex;
  std::string history;

  std::mutex clients_mutex;
  std::vector<std::shared_ptr<Client>> clients;
};
