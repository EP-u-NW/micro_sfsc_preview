## POSIX
To run the examples on a POSIX system, copy the `makefile` to the root of this repository and use the `make` command.
To chose an example to run, set the corresponding preprocessor directive in the makefile (in the `CPPFLAGS` variable).

## Windows cross-compile
You can cross-compile for Windows using mingw32.
Edit the makefile and comment in all occurrences of `x86_64-w64-mingw32-g++` and `x86_64-w64-mingw32-gcc`. Also, comment in `-lwsock32 -lws2_32` in `LDLIBS_AFTER`.