// try to comment the double inclusion guard: you will get an error
// when the file 4_include_guards.c includes this file twice
#ifndef INCLUDE_GUARDS_H
#define INCLUDE_GUARDS_H
// alternatively, you could use #pragma once, but it is not supported by all
// compilers 
// #pragma once 

// these header have their include guards, so can be safely included multiple
// times
#include <stdio.h>
#include <stdlib.h>

// preprocessor constant
#define TEST "test"

// macro function: this one replaces each text "TWICE(o)" with "2*o"
#define TWICE(o) 2*o 

// a structure with 4 ints:
typedef struct {
  int a,b,c,d;
} my_struct_t;

#endif // INCLUDE_GUARDS_H