#include <stdio.h>
#include "../defines.h"

int main(int argc, char **argv) {
  printf("Hello, world!\n");
  eprintf("  [MACRO] message %d", 10);
  wprintf("[MACRO] message");
  error("  [function] Message no args");
  error("  [function] Message with args: %d", 10);
  warning("[function] warning message al line %d", __LINE__);
  return 0;
}
