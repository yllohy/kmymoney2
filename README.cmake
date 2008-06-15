KMyMoney README.cmake
Author: Joerg Rodehueser <wampbier@users.sf.net>
Date  : June 8 2008

This README gives a short description how to build kmymoney with cmake.

-----------------------------------------------------
Precondition
-----------------------------------------------------

Check that cmake is installed on your machine and is in your PATH.
To do so, just type

$ cmake --version

on your command line. We need at least version 2.4.

-----------------------------------------------------
Out-of-source Build
-----------------------------------------------------

It is recommended, that you use a separate directory to build kmymoney2.
For example make a directory 'build' in your development environment.
This is completely in depended from the place where you have checked out
the source files.
Once you have created the directory 'cd' in there and type:

$ cmake <path_to_your_root_source_directory> -DCMAKE_INSTALL_PREFIX=<path_to_install_dir>

The root source directory is that directory where the first CMakeList.txt
file is stored. You can use relative paths to the source tree, too.
You have to call this only once, even if you change some CMakelist.txt files
cmake detect that and configured itself new.

The second directory is used to install kmymoney in. You should give cmake a directory
in your development environment to debug the program or even to test the installation.
If you leave that out, cmake will install kmymoney to /usr/local/.

-----------------------------------------------------
Start the build
-----------------------------------------------------

After cmake has finished, you have a set of ordinary makefiles in you directory.
You can type

$ make help

to see all the targets that are defined.

If you just type 

$ make

cmake builds the kmymoney (that's like $ make all).

You can also type

$ make install

to build and install kmymoney in the previous given install directory at once.
You find the executable in <install_directory>/bin .

-----------------------------------------------------
What is not working?
-----------------------------------------------------

I'm still working on the message extracting and merging. 
Also the unit test integration is not implemented now.
I expect that CMAKE is not working in all environments, which means
that the system inspection has to be improved.

Targets that are not implemented or should NOT be used are:

make dist
make distcheck
make package-messages
make merge-messages
make release
make debug
make check
make messages


