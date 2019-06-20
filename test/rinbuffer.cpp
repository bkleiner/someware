#include <iostream>

#include "util/util.h"

#define BUFFER_SIZE 32

int main(int argc, char const *argv[]) {
  ring_buffer<char> buf(BUFFER_SIZE);
  
  for (size_t i = 0; i < BUFFER_SIZE; i++) {
    buf.write(char(48 + i));
  }

  for (size_t i = 0; i < (BUFFER_SIZE / 2); i++) {
    char data = 0;
    if (buf.read(&data))
      std::cout << data << std::endl;
  }

  for (size_t i = 0; i < BUFFER_SIZE; i++) {
    buf.write(char(48 + i));
  }

  while (buf.count()) {
    char data = 0;
    if (buf.read(&data))
      std::cout << data << std::endl;
  }
  
  return 0;
}
