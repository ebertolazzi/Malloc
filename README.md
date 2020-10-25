submodule compilation
=====================

**Utils**

A collection of useful code for C++ applications:

- Terminal coloring use code from `https://github.com/agauniyal/rang`
- Stream compression use code from `https://github.com/geromueller/zstream-cpp`
- Stream formatting use code from  `https://fmt.dev`

in addition a TreadPool class, TicTic class for timing, Malloc class for easy allocation with traking of allocated memory.

COMPILE AND TEST
===============

**On linux**

~~~~
make clean
make
make run
~~~~

or using rake

~~~~
rake build_linux
~~~~

**On windows**

using MINGW on a bash shell

~~~~
make clean
make
make run
~~~~

or using Visual Studio

~~~~
rake build_win[2017,x64]
rake build_win[2017,x86]
~~~~

**On OSX use**

~~~~
make clean
make
make run
~~~~
