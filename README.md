submodule compilation
=====================

**Utils**

COMPILE
=======

**On linux**

~~~~
make clean
make
make install_local
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
make install_local
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
make install_local
~~~~
