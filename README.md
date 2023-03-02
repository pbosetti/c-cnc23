# C-CNC 2023
![CMake build](https://github.com/pbosetti/c-cnc23/actions/workflows/cmake.yml/badge.svg)

This is the repository of the code developed during the _Digital Manufacturing_ course, academic year 2022-23, Department of Industrial Engineering, University of Trento.

## Contents

* `examples`: contains introductory code examples
* `src`: contains the main project source code (library files)
* `src/main`: contains executables code
* `goodies`: useful stuff
* `products`: destination folder for cross-compiled binaries

## Prerequisites

We are going to develop a C project for the UNIX command-line environment. On Linux and MacOS, you are set already, on Windows we are going to use a WSL2 environment with Ubuntu OS (more later).

If you have a Windows laptop, you are suggested to use at least Windows 10, preferably Windows 11. Be sure to install Windows Terminal app <https://learn.microsoft.com/en-us/windows/terminal/install>

Regardless your platform, begin with installing Visual Studio Code from here: <https://code.visualstudio.com/download>. Then open a terminal and type the following to install commonly used VSCode extensions:

```sh
code --install-extension xaver.clang-format
code --install-extension tintinweb.graphviz-interactive-preview
code --install-extension canna.figlet
code --install-extension Juancete.gcode-formatter
code --install-extension vscode-gcode.gcode
code --install-extension vadimcn.vscode-lldb
```

Also, you want to install the Fira Code font family, from <https://github.com/tonsky/FiraCode/releases/tag/6.2>. Download the file `FiraCode_6.2.zip`, unzip it and install the new font.

## VS Code setup

The development is carried out in Visual Studio Code (VS Code for brevity).

I suggest to configure VS Code with the following settings. Open the settings file: `Ctrl`+`Shift`+`p` then type `json` and select the item "Preferences: Open Settings (JSON)". Then be sure that the list contains the following items (the first two settings **only if you have installed Fira Code font**):

```json
{
  "editor.fontFamily": "Fira Code",
  "editor.fontLigatures": true,
  "editor.tabSize": 2,
  "editor.insertSpaces": true,
  "editor.wrappingIndent": "indent",
  "editor.renderControlCharacters": true,
  "editor.wordWrap": "bounded",
  "editor.wordWrapColumn": 80,
  "editor.rulers": [80],
  "editor.renderLineHighlight": "all",
  "cmake.configureOnEdit": false,
  "cmake.configureOnOpen": false,
  "C_Cpp.default.cppStandard": "c++17",
  "C_Cpp.default.cStandard": "c17",
}
```

It there are already other items in the JSON file, just add (don't replace) the above ones to the list (pay attention to separate each line with a comma and to put everithyng in between the outer curly braces).


### 🪟 Windows (or 🐧 Ubuntu/Debian Linux)
The project must be built with a linux toolchain. On Windows, we are using a WSL2 environment with Ubuntu OS. To install it, follow the instructions at <https://docs.microsoft.com/en-us/windows/wsl/install-win10>. 

To enable the compilation we need to install a few packages: on the linux console, type:

```bash
sudo apt install build-essential make cmake cmake-curses-gui clang clang-format lldb libgsl-dev ruby figlet sshfs
sudo gem install gv_fsm
sudo update-alternatives --set c++ /usr/bin/clang++
sudo update-alternatives --set cc /usr/bin/clang
```

### 🍎 MacOS
You need to have Xcode installed: do that through the App Store and—once finished—launch Xcode and accept the licence terms. Then you can close it.

On MacOS, the command equivalent to `apt` is `brew`: you have to install it by following the instructions on <https://brew.sh>, which means to type the following in the Terminal.app:

```sh
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

Then **close the terminal** and open a new one and proceed as follows:

```sh
brew install figlet gsl clang-format graphviz gpg
brew install --cask cmake
curl -sSL https://rvm.io/mpapis.asc | gpg --import -
curl -sSL https://rvm.io/pkuczynski.asc | gpg --import -
curl -sSL https://get.rvm.io | bash -s stable --auto-dotfiles
```

Close and open a new terminal, again, then:

```sh
rvm install ruby-2.7
gem install gv_fsm
```


## Build with Cmake

Building a project with Cmake is a two-step process. The first step is called *configuration*, and it results in populating the `build` folder with all the contents needed for the compilation. The second step is called *compilation* and results in the products of the build to be created in the root of the `build` folder. There is an optional third step, *install*, that copies the build products into a destination folder. This project is configured to have the local `bin` forder as destination.

1. (configuring) from the terminal, be sure to be in the project's toot directory and then issue the command `cmake	-Bbuild`: this means configure the project in the `build` directory, searching for the `CMakeLists.txt` file in the current directory
2. (compilation) from the terminal, compile the project with the command `make -C build` 
3. (optional install) if you want to install the build products, type `make -C build install`: this copies binaries into the `bin` and `lib` folders of the root project folder

**Note 1.**: the `cmake` command must be run the first time, and then every time that you create, move, or rename source files. Conversely, if you only change contents of source files, then you only need to `make`. The `make` command is smart enough not to recompile files that have been already compiled and that are unchanged from the previous build: this reduces a lot the compilation time for large projects. The option `-Cbuild` (the space is optional) tells make to work in the directory `build`.

**Note 2.**: the command `make` takes as optional argument the name of the _target_ to build, i.e. the list of products to be generated. A special target is `all`, so `make all` means "let's build everything". `all` is also the default target, so if you do simply `make`, then you are building everithing. Other useful targets are `clean` (for removing previously generated binaries) and `install` (for copying the binaries into the destination folder). The available targets are listed by the special target `help`: `make -Cbuild help`.

**Note 3.**: the command `make install` also does the compilation **if needed**, so if you want the products in the install folder just call `make install` (i.e. there is no need for calling `make` and then `make install`)

For brevity sake, after having configured the project for the first time, in the following you can do everithing with one single command: `cmake --build build -t install`: this is doing, in sequence, step 1 (only if the `CMakeLists.txt` file has changed), then step 2 (only if sources have changed), then step 3. In the latter command, `--build` is an option that takes one argument, the build folder, which is named `build`; the second option, `-t`, takes as argument the name of the build _target_: by default it is `all` (meaning, "build all targets"), and thus `-t install` means "build the target ` install`" (which implies the target `all`).

## Execute the compiled binaries

The command `cmake --build build` compiles the binary executables under `build`. Those binaries are compiled with minimum optimizations and contain the debug symbols, i.e. they are suitable for debugging. From the project root directory, they can be run as `build/ini_test` (for example).

The command `cmake --build build -t install` (or `cmake --install build`) also installs optimized versions under `products_host`: executables go under `bin` and libraries under `lib`. These files are speed-optimized and **cannot be debugged**. From the project root directory they can be run as `products_host/bin/ini_test`.

You are suggested to run `export PATH=$PATH:$PWD/products_host/bin` once per session, so that you can simply run a program by typing its name (e.g. `ini_test`).


## Author

Paolo Bosetti (`paolo dot bosetti at unitn dot it`).

## Acknowledgments

This project uses the `tomlc99` library <https://github.com/cktan/tomlc99>.

## License

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><br />This work is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>.