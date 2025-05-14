#pragma once
#include <arpa/inet.h>
#include <cassert>
#include <functional>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

/** @class Server
 * @brief A TCP server
 *
 * Starts a TCP server that listens on port 2137. When a client connects its
 * socket is handed to the `client_handler`, that will run on a separate thread.
 */
class Server {
public:
  /**
   * @brief Setups a socket
   *
   * Creates an INET socket, binds it to `0.0.0.0:2137` and starts listening
   * for maximum of 127 clients.
   *
   * @param client_handler Function responsible for communicating with a client.
   * It takes an `int` argument that is an socket of a client. This function
   * should close the socket when returning.
   */
  Server(std::function<void(int)> client_handler, const std::string &address,
         int port)
      : client_handler{client_handler} {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(server_fd != -1);

    {
      int optval = 1;
      int ret = setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &optval,
                           sizeof(optval));
      assert(ret != 1);
    }

    {
      sockaddr_in in;
      in.sin_family = AF_INET;
      // in.sin_addr.s_addr = htonl(INADDR_ANY);
      inet_pton(AF_INET, address.c_str(), &in.sin_addr);
      in.sin_port = htons(port);
      int ret = bind(server_fd, (sockaddr *)&in, sizeof(in));
      assert(ret != -1);
    }

    {
      int ret = listen(server_fd, 127);
      assert(ret != 1);
    }
  }

  /** @fn Server::~Server()
   *
   * Waits for all clients to end connections and closes a server socket.
   */
  ~Server() {
    for (auto &thread : clients) {
      thread.join();
    }

    close(server_fd);
  }

  /** @fn void Server::accept_connection()
   *
   * Accept one new connection.
   * Run a new thread for this connection with `client_handler`.
   */
  void accept_connection() {
    int client_fd = accept(server_fd, NULL, 0);
    assert(client_fd != -1);

    clients.emplace_back(std::bind(client_handler, client_fd));
  }

private:
  int server_fd;

  std::vector<std::thread> clients;
  std::function<void(int)> client_handler;
};
