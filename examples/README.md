# C programming exercises

## Notes and caveats

* Remember to save the file before compiling (CTRL-S)
* Respect proper indentations (2 spaces, inserted by hitting the Tab key; increase indentation by one level for each nested block)

## Libraries

### How to create a _static_ library

First, compile all the C source files containing functions that you want to include in your library:

```sh
gcc -c f1.c f2.c
```

This creates `f1.o` and `f2.o` (object files)

Second, archive the object files into a single library file:

```sh
ar rcs mylib.a f1.o f2.o
```

Third, write a header file that contains the declarations of all the functions in the library (only the signature of each function), and `#include` that header in your main file.

Lastly, compile the main file by passing to the compiler also the library name:

```sh
gcc ex1.c mylib.a -o ex1
```

### How to create a _shared_ (or _dynamic_) library

A shared library is a piece of software that remains in a different file w.r.t. the executable, so that many different executables can _share_ the same library. On the opposite, a static library is merged to the executable into a single file on compilation.

To create the dynamic library with `f1.c` and `f2.c`:

```sh
gcc -shared -fPIC src/f1.c src/f2.c -o libmylib.so
```

This creates the library with the standard name of `libmylib.so` (`.so` stands for _shared object_). The library must be copied into the system folder in order for the operating system to find it on launch of the executable:

```sh
sudo cp libmylib.so /usr/lib
```

The executable can be _linked_ with the library during compilation:

```sh
gcc src/ex1.c -L. -lmylib -o ex1d
```

where `-L.` instructs the compiler to search for shared libraries in the local folder (`.`), and `-lmylib` links to the `lib`*`mylib`*`.so` library file.

## How to debug an executable

Visual Studio Code can be configured to debug your executable. To do so, you need to perform three steps:

1. (re)compile your code with the `-g` switch, e.g.: `gcc src1.c src2.c src3.c -omyexec -lm -g`
2. Setup the debugger: switch to the debugging environment (click on the bug icon on VSC toolbar on the left); click on the little gear button on the top left panel; in the file `launch.json` that opens, change the `"program"` field to point to your executable, as in `"${workspaceFolder}/examples/poly"`, where workspaceFolder represents the root folder that you opened in VSC; save and close `launch.json`
3. Set at least one breakpoint in your code: move the mouse pointer slightly to the left of a line number until you see a dark red circle, and click it.

Now you can run the program in the debugging environment, inspecting the content of variables and moving one line at a time by clicking on the buttongs *step over* (skips calls to functions), *step into* (follows calls to functions), and *step out* (completes the current function and returns to the caller one).