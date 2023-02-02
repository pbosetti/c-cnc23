# Things to remember for C Programming

# Compilation

Here is a list of the most commonly used switches of the `clang` command for compiling simple programs (for more complex ones, use cmake and make):

* `-o filename`: specify the name of the executable file to be created
* `-g`: generate a debuggable executable
* `-Ipath`: specify the folder where to search for headers
* `-Lpath`: specify the folder where to search for non-system libraries
* `-llibname`: link to the library named `libname`, e.g `-lm` for linking to the math library (whose name is `m`)

Apart these flags, you must also provide the list of source files to be compiled (separated by spaces).

# Pointers

Pointers are special variables that hold the address of a memory location rather
than a value.

Pointers are *declared* by prepending a `*` to their name:

```c
double *val;
```

To deal with pointers and memory addresses there are to operators: the
*reference* operator `&` (which returns the address of a variable) and the
*dereference* operator `*` (which returns the content of a pointer). So you can
assign the address of a variable to a pointer like this: 

```c
double *val_p;
double val = 5.9;
val_p = &val;
```

and you can get the value stored at the memory location pointed by a pointer
like this:

```c
printf("Content of %p is %f\n", val_p, *val_p);
```

To help your memory, remember that the reference operator `&val` is pronunced as
"address of `val`", the dereference operator `*val_p` is pronunced as "content
of `val_p`".

# Arrays

## Array declaration

An array can be declared in the stack memory or on the heap memory. 

* *on the stack*: the array or `n` elements is declared as: `double ary[n];`
* *on the heap*: the array is declared as: `double *ary;` then it must be
  assigned to a memory block allocated with proper size via a call to `malloc()`

In any case (stack or heap) in C an array is a pointer to its first element, so
in both cases, `ary` points to the memory location containing the first value of
the array. The square brackets operator is actually asking for the content of a
memory location with a given offset respect to the first element. So:
`
```c
double ary[3] = {1, 2, 3};
ary; // memory location containing 1
*ary; // content of memory location containing 1 (i.e., value of 1)
ary[2]; // content of the memory with offset 2 from ary (i.e., value of 3)
ary + 1; // memory location containing 2
*(ary + 1); // content ofthe previous, value of 2
```

The first line is a **quick initialization**: see [later
on](#quick-initialization) for further information on this.

The last two lines are an example of **pointer arithmetics**: incrementng or
decrementing a pointer allows to read memory contents at a given offset from the
original pointer. As a matter of fact, `ary[n]` and `ary + n` are exactly the
same thing, just with different syntax.

## When to use stack or heap declared arrays

There are two reasons for selecting stack or heap declarations:

1. array size
2. array scope

**Array size**: the stack is property of the function where the array is
declared, and it has a limited size (typically about 8 Mb per function), but it
is a faster memory. Conversely, the heap capacity is only limited by the system
total memory, but it is slower. So small arrays are beter declared on the stack,
while large arrays have to be declared on the heap. Moreover, a stack array is
*static*: you cannot change its size after declaration. Conversely, an heap
array is *dynamic* and can be reallocated to a bigger (or smaller) size if
needed.

**Array scope**: stack array only live in the stack, which is deleted upon
function exit. So if you need to pass an array to other functions you have to
declare it on the heap.

# Custom types and Compound types

## Custom types

The C language allows to define custom types as *alias* to existing types with
the `typedef` keyword:

```c
typedef data_t double;
```

With such a definition I can now use the type `data_t` for variables
representing real values. This is handy, because if in a later moment I want to
use `float` types (in order to save some memory, for example), I can simply
change the `typedef` (one single change) rather than cange all the declarations
from `double` to `float` (possibly tens or hundreds of changes).

Typedef is also used for providing platform-independent types as those provided
by the header `stdint.h`:

```c
#include <stdint.h>
...
uint16_t a; // unsigned 16 bit integer
int32_t b;  // signed 32 bit integer
```

Unlike native types (as `long int`) whose actual nit size depends on the
architecture, those derived types always have the given, platform independent
size.

## Compound types

A C *structure* is a compound type: a collection of fields of different types
representing a complex object.

A structure is *defined* with the keyword `struct`: the general definition is
`struct <name> { type field1; type field2; ...}`. For example:

```c
struct my_struct {
  int a, b;
  double c;
  char *s;
  double *array;
}; // note the closing semicolon
```

An instance of this structure can be then declared and used as:

```c
struct my_struct ms1;
struct my_struct *ms2;
...
ms1.a = 1;
ms1.c = 2.3;
ms2->a = 2;
ms2->b = 9;
```

So, if a struct is declared as a normal variable its field are accessible as
`struct_name.field_name` (dot notation). If the struct is declared as a pointer
its fields are accessible as `struct_name->field_name` (arrow notation).

Now, `struct my_struct` represents a valid *type*. So it is possible to alias it
to a shorter name by using typedef:

```c
typedef struct my_struct {
  //fields
} mystruct_t;
...
mystruct_t ms; // this is an instance of struct my_struct
```

When declaring structures as typedef, the structure name becomes optional, and
you can omit it. The last block becomes:

```c
typedef struct { // no structure name here!
  //fields
} mystruct_t;
...
mystruct_t ms; // this is an instance of struct my_struct
```

## Quick initialization

Arrays and structs can be declared and initialized in one step assigning a list
of values in curly braces. The size of the list must be less or equal the number
of elements in the array or struct:

```c
typedef struct {
  double a, b; 
  int c, 
  char name[100];
} mystruct_t;
...
double a[5] = {1, 2, 3, 4, 5};
mystruct_t s1 = {1.0, 3.0, 20, "test"};
mystruct_t s2 = {5.0, 2.0};
```

Note, though, that this direct assignment of multiple values in a list **only
works on variable declaration**: you cannot directly assign a compound object
after its declaration. Moreover, this direct assignment **does not work** if the
compound type is declared as a pointer (e.g. `mystruct_t *s3`).  

A constant list or a constant string used in quick initialization is called a **literal** value.

## Where to define custom types and structures

Definition of custom types and structures can be made either at the top level or
within a function. **New types are only visible within the local context**. This
means that if you define a `struct` within a function, then you can only declare
variables of that type within the same function. Conversey, if you define a
custom type (including `struct`s) at the top level, that definition is available
in every function.

As a rule of thumb:

* things that are of common usage have to be defined in header files (`.h`); in
  this way they end up being defined at the top level of *each source file*
  (`.c`) that includes that header.
* things that are of local usage have to be defined in source files (`.c`): at
  the top level if they are used in more than one function, within a single
  function in any other case.

## Naming schemes

* Variable and function names shall be declared as: `my_variable`
* Global variables (declared at the top level) shall be declared as:
  `MyGlobalVariable`
* custom types (including `struct` types) shall be *defined* as: `mystruct_t` 

# Array of structs

In C it is possible to create array of every possible type, including structs:

```c
typedef struct {
  double x, y;
} point_t;
...
point_t *points = malloc(n * sizeof(point_t));
for (i = 0; i < n; i++) {
  points[i].x = ary_x[i];
  points[i].y = ary_y[i];
}
```
