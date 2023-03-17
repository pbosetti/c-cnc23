#include <stdio.h>
#include "../defines.h"

int main(int argc, char **argv) {
  printf("Hello, world!\n");
  printf("version %s (%s build)\n", VERSION, BUILD_TYPE);
  eprintf("  [MACRO] message with argument %d", 10);
  wprintf("[MACRO] message, no argument");
  return 0;
}
