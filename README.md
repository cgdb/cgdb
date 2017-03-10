# CGDB

CGDB is a very lightweight console frontend to the GNU debugger.  It provides
a split screen interface showing the GDB session below and the program's
source code above.  The interface is modelled after vim's, so vim users should
feel right at home using it.

Screenshot, downloads, and documentation are available from the home page:
http://cgdb.github.com/

Official source releases are available here:
http://cgdb.me/files

## Build Instructions

### Dependencies

You must have the following packages installed.
- sh
- autoconf
- automake
- aclocal
- autoheader
- libtool
- flex
- bison
- gcc/g++

### Preparing the configure

Run ./autogen.sh in the current working directory to generate the configure
script.

### Running configure, make and make install

You can run ./configure from within the source tree, however I usually run
configure from outside the source tree like so,
```
mkdir ../build
cd ../build
../cgdb/configure --prefix=$PWD/../prefix
make -srj4
make install
```

I typically enable more error checking with the build tools like so,

> YFLAGS="-Wno-deprecated" CFLAGS="-std=c11 -g -O0 -Wall -Wextra -Wshadow -pedantic -Wno-unused-parameter" CXXFLAGS="-std=c++11 -g -O0 -Wall -Wextra -Wshadow -Werror -pedantic -Wmissing-include-dirs -Wno-unused-parameter -Wno-sign-compare -Wno-unused-but-set-variable -Wno-unused-function -Wno-variadic-macros" ../cgdb/configure --prefix=$PWD/../prefix

If you like to have a silent build, and the libtool link lines are bothering
you, you can set this environment variable to suppress libtools printing of
the link line,
>  LIBTOOLFLAGS=--silent
