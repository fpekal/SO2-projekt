#include "server.h"

void random_func(int a) {
  const char str[] = "HTTP/1.1 200 OK\n"
                     "Connection: Closed\n"
                     "Content-Type: text/html\n"
                     "Content-Length: 8\n\n"
                     "Tekscior";

  write(a, str, sizeof(str));
}

int main(int argc, char *argv[]) {
  Server server{random_func};

  while (true) {
    server.accept_connection();
  }

  return 0;
}
