#pragma once
#include <cassert>
#include <functional>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

class Server {
public:
  Server(std::function<void(int)> client_handler)
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
      in.sin_addr.s_addr = htonl(INADDR_ANY);
      in.sin_port = htons(2137);
      int ret = bind(server_fd, (sockaddr *)&in, sizeof(in));
      assert(ret != -1);
    }

    {
      int ret = listen(server_fd, 127);
      assert(ret != 1);
    }
  }

  ~Server() {
    for (auto &thread : clients) {
      thread.join();
    }

    close(server_fd);
  }

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
