#pragma once
#include <algorithm>
#include <functional>
#include <memory>
#include <mutex>
#include <signal.h>
#include <stdexcept>
#include <string>

#include "server.h"

// Trim from start (in-place)
static std::string ltrim(std::string s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
          }));
  return s;
}

// Trim from end (in-place)
static std::string rtrim(std::string s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](unsigned char ch) { return !std::isspace(ch); })
              .base(),
          s.end());

  return s;
}

// Trim from both ends (in-place)
static std::string trim(std::string s) {
  s = ltrim(s);
  return rtrim(s);
}

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
  /**
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

/** @class Chat
 * @brief A chat server with only one channel and no security whatsoever.
 *
 * It's a wrapper around a `Server` class.
 * When a client connects to the server, it sends him the whole message history
 * and adds him to the list of connected clients. When a client sends a message,
 * server receives it and broadcast it to all clients, including the one that
 * sent it in the first place.
 */
class Chat {
public:
  /**
   * @brief Construct a `Chat` object.
   *
   * Initializes a `server` member with `Chat::client_handler`.
   */
  Chat(const std::string &address, int port)
      : server{std::bind(&Chat::client_handler, this, std::placeholders::_1),
               address, port} {
    signal(SIGPIPE, SIG_IGN); // Make sure the process won't crash
  }

  /** @fn void Chat::run()
   * @brief Start accepting clients.
   *
   * Blocking function. It enters an infinite loop and accepts all pending
   * connections to the server.
   */
  void run() {
    while (true) {
      server.accept_connection();
    }
  }

private:
  /** @fn void Chat::client_handler(int fd)
   * @brief Handles an entire connection with a client.
   *
   * Adds and removes a client from a list of all clients.
   * Firstly, it sends a client all of the chat history and then it listens for
   * client's messages and broadcast them.
   */
  void client_handler(int fd) {
    auto client = add_client(fd);

    auto history = get_history();
    client->send(history);

    while (true) {
      try {
        auto new_message = trim(client->receive()) + "\n";

        add_history(new_message);

        send_to_all(new_message);
      } catch (std::runtime_error &e) {
        break;
      }
    }

    remove_client(client);
  }

  /** @var Server Chat::server
   * @brief TCP server on which the chat server is based.
   */
  Server server;

  /** @fn std::string Chat::get_history()
   * @brief Get whole chat history
   *
   * It's thread-safe.
   */
  std::string get_history() {
    std::unique_lock<std::mutex> lock{history_mutex};

    return history;
  }

  /** @fn void Chat::add_history(const std::string &message)
   * @brief Add a message to the history
   *
   * It's thread-safe.
   * The message is stripped from its last character, that is newline character.
   */
  void add_history(const std::string &message) {
    std::unique_lock<std::mutex> lock{history_mutex};

    history += trim(message);
    history += "\n";
  }

  /** @fn std::shared_ptr<Client> Chat::add_client(int fd)
   * @brief Add a client to the list of all clients.
   *
   * It's thread-safe.
   *
   * @return shared_ptr to the newly created `Client` object.
   */
  std::shared_ptr<Client> add_client(int fd) {
    auto client = std::make_shared<Client>(fd);

    std::unique_lock<std::mutex> lock{clients_mutex};
    clients.push_back(client);
    return client;
  }

  /** @fn void Chat::remove_client(std::shared_ptr<Client> client)
   * @brief Removes the client from a list of all clients.
   *
   * It's thread-safe.
   * When there is no such client, no error is returned;
   */
  void remove_client(std::shared_ptr<Client> client) {
    std::unique_lock<std::mutex> lock{clients_mutex};
    auto iter = std::find(clients.begin(), clients.end(), client);
    clients.erase(iter);
  }

  /** @fn void Chat::send_to_all(const std::string &message)
   * @brief Sends a message to all clients in the list.
   *
   * It's thread-safe.
   */
  void send_to_all(const std::string &message) {
    std::unique_lock<std::mutex> lock{clients_mutex};

    for (auto client : clients) {
      client->send(message);
    }
  }

  /** @var std::mutex Chat::history_mutex
   * @brief Locks an access to the `history` member.
   */
  std::mutex history_mutex;
  /** @var std::string Chat::history
   * @brief All messages that were sent through a server.
   */
  std::string history;

  /** @var std::mutex Chat::clients_mutex
   * @brief Locks an access to the `clients` list.
   */
  std::mutex clients_mutex;
  /** @var std::vector<std::shared_ptr<Client>> Chat::clients
   * @brief Stores a list of all clients connected to the server at the moment.
   *
   * Using `send_to_all` function sends a message to all clients inside of this
   * list.
   */
  std::vector<std::shared_ptr<Client>> clients;
};
