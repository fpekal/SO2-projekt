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
 * Class allowing for sending and receiving messages from clients.
 * Closes file descriptor when the destructor is called.
 */
class Client {
public:
  Client(int fd) : fd{fd} {}
  ~Client() { close(fd); }

  void send(const std::string &message) {
    std::unique_lock<std::mutex> lock{send_mutex};

    int ret = write(fd, message.c_str(), message.length());
    if (ret == -1) {
      throw std::runtime_error{"Client disconnected"};
    }
  }

  std::string receive() {
    char buf[1024];

    int len = read(fd, buf, 1023);
    if (len == 0)
      throw std::runtime_error{"Client disconnected"};

    buf[len + 1] = 0;

    return buf;
  }

private:
  std::mutex send_mutex;
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
