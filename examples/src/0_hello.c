//   _   _      _ _         __        __         _     _ _ 
//  | | | | ___| | | ___    \ \      / /__  _ __| | __| | |
//  | |_| |/ _ \ | |/ _ \    \ \ /\ / / _ \| '__| |/ _` | |
//  |  _  |  __/ | | (_) |    \ V  V / (_) | |  | | (_| |_|
//  |_| |_|\___|_|_|\___( )    \_/\_/ \___/|_|  |_|\__,_(_)
//                      |/                                 
// To create big text as above, in VSC select a line, type Ctrl-Shift-p, type
// "fig" and then select "FIGlet: create ASCII art text": the selected text will
// become bigger. Then, without changing the selection, type Ctrl-Shift-7
// (italian keyboard) and all the selectd lines get commented

#include <stdio.h>
#define GREET "Hello, World!"

int main() {
  printf(GREET "\n"); // note *
  return 0;
}

// * - This "string" is called a string literal. When two string literals are
//     placed next to each other (with no commas in between!) the compiler
//     simply merges them. So "Hello, World!" "\n" becomes "Hello, World!\n".

// now run this command:
//   clang -E examples/src/1_hello.c | code -
// -E: means "preprocess only"
// |: means pass the output (or "pipe") to the next command (code)
// code -: means open a new feil with the output coming from previous command
//
// This will open a temporary file with the source after preprocessor.
// You may notice the whole stdio.h included at the begining, 
// also, the GREET preprocessor constant has been replaced in line 12.
// Lines starting with "# " are not compiled and are tags representing 
// preprocessor inserion traces