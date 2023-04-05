#include <stdio.h>
#include "../defines.h"

int main(int argc, char **argv) {
  printf("Hello, " GRN "world!" CRESET "\n");
  printf("Version: " VERSION ", build type " BUILD_TYPE "\n");
  // printf("Version %s, build type %s\n", VERSION, BUILD_TYPE);
  wprintf("This is the %dst warning that I'm givig you!\n", 1);
  eprintf("This is the %dnd warning that I'm givig you!\n", 2);
  return 0;
}
