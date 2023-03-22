//   ___            _           _                                  _     
//  |_ _|_ __   ___| |_   _  __| | ___    __ _ _   _  __ _ _ __ __| |___ 
//   | || '_ \ / __| | | | |/ _` |/ _ \  / _` | | | |/ _` | '__/ _` / __|
//   | || | | | (__| | |_| | (_| |  __/ | (_| | |_| | (_| | | | (_| \__ \
//  |___|_| |_|\___|_|\__,_|\__,_|\___|  \__, |\__,_|\__,_|_|  \__,_|___/
//                                       |___/                           
#include "include_guards.h" 
#include "include_guards.h" 

// support function: it might be DEFINED here (before usage)
// or rather DECLARED here and defined after the main() (after usage)
void print_args(int argc, const char **argv) {
  printf("Number of args: #%i\n", argc);
  int i;
  for(i=0; i<argc; i++){
    printf("Args #%i is: %s\n", i, argv[i]);
  }
}

int main(int argc, const char **argv) {
  print_args(argc, argv);

  if(argc == 1) {
    printf("I don't have enough arguments!\n");
    return 0;
  }

  double a = atof(argv[1]);

  printf("Symbol test is: %s\n", TEST);
  // but TEST is a literal string, so this also works:
  printf("Symbol test is: " TEST "\n"); // and it is faster ;)

  printf("Test TWICE(%f): %f\n", a, TWICE(a));

  return 0;
}