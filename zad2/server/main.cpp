#include <iostream>

#include "chat.h"

int DEFAULT_PORT = 2137;
std::string DEFAULT_ADDRESS = "127.0.0.1";

int main(int argc, char *argv[]) {
  int port = DEFAULT_PORT;
  std::string address = DEFAULT_ADDRESS;

  if (argc == 2) {
    std::string arg1 = argv[1];
    if (arg1 == "--help" || arg1 == "-h") {
      std::cout << "Usage:\n" << argv[0] << " [address port]\n";
      return 0;
    }
    std::cerr << "Wrong arguments.\n";
    return 1;
  }
  if (argc == 3) {
    try {
      port = std::stoi(argv[2]);
      address = argv[1];
    } catch (...) {
      std::cerr << "Wrong arguments.\n";
      return 1;
    }
  }
  if (argc > 3) {
    std::cerr << "Too many arguments.\n";
    return 1;
  }

  Chat chat(address, port);

  chat.run();

  return 0;
}
