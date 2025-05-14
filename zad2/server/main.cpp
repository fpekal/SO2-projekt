#include "chat.h"

int main(int argc, char *argv[]) {
  Chat chat("127.0.0.1", 2137);

  chat.run();

  return 0;
}
