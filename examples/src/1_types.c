//   _____
//  |_   _|   _ _ __   ___  ___
//    | || | | | '_ \ / _ \/ __|
//    | || |_| | |_) |  __/\__ \
//    |_| \__, | .__/ \___||___/
//        |___/|_|

#include <float.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// We are going to use:
// - ALL_CAPITALS for preprocessor constants
// - snake_case for variable and function names
// - CamelCase for global variables

// GLOBAL VARIABLES
// They shall be avoided as much as possible:
int MyCounter = 0;

// GLOBAL CONSTANTS
// their value cannot be changed (compile-time error)
const double PiApprox = 3.14159265359;
// as an alternative, we can use preprocessor constants:
#define PI_APPROX 3.14159265359L
// on bare-metal microprocessors the latter is preferable, for the associated
// memory is in the program space rather in the variable space, which is
// typically limited and precious. On standard computers it makes little, if
// any, difference.
// 3.14159265359L is called a LITERAL. The length specifier (suffix char) can be
// L (for double size) and/or U for unsigned, only valid for integers

// MAIN FUNCTION
// The mandatory main function must:
// - return an integer value (int): 0 means success, any other value failure
// - two arguments: the number of command line arguments and a vector of strings
//   holding the command line arguments
int main(int argc, char const **argv) {
  // NOTE: exceptionally, and for the sake of clarity, here we will declare
  // variables when needed; in real applications it is preferable to declare
  // variables at the beginning of each block, fore some compilers (notably
  // Visual Studio for C) do not accept late declarations.

  // INTEGERS
  // native integer is int: it takes 32 bits (4 bytes) on most 64-bit CPUs
  printf("INTEGER TYPES\n");
  int my_int = INT_MAX;
  printf("int: size is %lu, my_int = %d\n", sizeof(my_int), ++my_int);
  printf("int: size is %lu, my_int = %d\n", sizeof(int), my_int++);
  printf("int: size is %lu, my_int = %d, INT_MIN = %d\n", sizeof(int), my_int,
         INT_MIN);
  // Shorter types:
  short int my_short = 10;
  char my_char = 10;
  printf("size of my_short is %lu (max %d), size of my_char is %lu (max %d)\n",
         sizeof(my_short), INT16_MAX, sizeof(my_char), CHAR_MAX);
  // Longer types:
  long int my_long = LONG_MAX; // on some platforms this is shorter
  long long int my_longlong = LLONG_MAX;
  printf("size of my_long is %lu (max %ld), size of my_longlong is %lu (max "
         "%lld)\n",
         sizeof(my_long), LONG_MAX, sizeof(my_longlong), LLONG_MAX);
  // unsigned types have the same size as signed ones, but shift all values in
  // the positive range
  unsigned int my_uint = UINT_MAX;
  printf("unsigned int: size is %lu, max is %u, my_uint = %d\n",
         sizeof(my_uint), UINT_MAX, ++my_uint); // overflow to 0

  // The actual size of the above types is platform dependent. 32 bit CPUs may
  // have different sizes, as well as 128 bit CPUs or 16 bit microcontrollers.

  // When needed and for portability of code, the header stdint.h makes
  // available explicit int types, which always have the same length:
  uint16_t my_u16 = UINT16_MAX;
  printf("size of uint16_t is %lu\n", sizeof(my_u16));

  // FLOATING POINT
  printf("\nFLOATING POINT TYPES\n");
  // floating point values follow IEEE standard 754 and have the same size
  // regardless the platform
  float my_float = PiApprox;
  double my_double = PI_APPROX;
  long double my_ldouble = PI_APPROX;

  printf("my_float: %f, size %lu\n", my_float, sizeof(my_float));
  printf("max: %e, min: %e, digits: %d, epsilon: %e\n", FLT_MAX, FLT_MIN,
         FLT_DIG, FLT_EPSILON);
  printf("my_double: %e, size %lu\n", my_double, sizeof(my_double));
  printf("max: %e, min: %e, digits: %d, epsilon: %e\n", DBL_MAX, DBL_MIN,
         DBL_DIG, DBL_EPSILON);
  printf("my_ldouble: %Lf, size %lu\n", my_ldouble, sizeof(my_ldouble));
  printf("max: %Le, min: %Le, digits: %d, epsilon: %Le\n", LDBL_MAX, LDBL_MIN,
         LDBL_DIG, LDBL_EPSILON);

  // Problem: if you start using float for all variables and unsigned int for
  // all counters, and later on you realize that these types are insufficient,
  // you have to catch and change a lot of declarations (long, tedious and error
  // prone). So, following the DRY rule it is preferable to use typdefs:
  typedef float data_t;
  typedef uint16_t index_t;
  // now you have two custom types to use for declaring variables:
  data_t var1 = 0;
  index_t counter1 = 0;
  // for counters and vector sizes, there is the size_t type in stdio.h

  // GUIDELINES: 
  // * be sure of how much capacity you need for different types of variables,
  //   then use typedefs in a main header that you will always include.
  // * If you plan to port the code on different architectures, it might be a
  //   good idea to use explicit types (e.g. typedef uint32_t index_t).
  // * Remember that the native integer type, int, it is also the fastest one:
  //   when you know that you have a short loop, whose index will always fit an
  //   int regardless the architecture, then declare the index as int, for the
  //   CPU can manage it as fast as possible, while other type might require
  //   more CPU cycles. This is true regardless the signedness.

  // STRINGS
  // Strings in C are arrays of bytes. One byte is an unsigned char. To
  // mark the end of a string, the last character must be a zero, aka NULL
  // CHARACTER or '\0'.
  // Being arrays, strings must be declared with their capacity.
  // String literals are delimited with double quotes ("string").
  // A string literal "string" implicitely includes the trailing null.
  // Char literals are delimited with single quotes ('a', '\n', '\0').
  printf("\nSTRINGS\n");
  char my_string[128] = "Hello, World!";
  unsigned char my_c = 'A';
  my_c++;
  printf("my_string: \"%s\", capacity: %lu, length: %lu\n", my_string,
         sizeof(my_string), strlen(my_string));
  // Note: strlen() does not count the trailing \0
  printf("my_c: '%c', ASCII value: %u\n", my_c, my_c);
  // Remember: an array in C is a pointer to its first element. So an array of
  // strings is an array of arrays, thus it is declared as a double pointer (as
  // in char const **argv argument of main)

  return 0;
}