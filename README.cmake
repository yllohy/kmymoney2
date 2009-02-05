KMyMoney README.cmake
Author: Joerg Rodehueser <wampbier@users.sf.net>
    and Holger <yllohy@googlemail.com>
Date  : June 8 2008
    and October 16 2008

This README briefly describes how to build kmymoney with cmake.

-----------------------------------------------------
Quickstart 0: Precondition
-----------------------------------------------------

Check that cmake is installed on your machine and is in your PATH.
To do so, just type

$ cmake --version

on your command line. Version 2.4 is required, the most recent
stable version of cmake is preferred.

-----------------------------------------------------
Quickstart 1: Start using cmake, now
-----------------------------------------------------

Go to the top level of your working directory. Now just type

$ cmake .
    to generate the Makefiles.
$ ccmake .
    to change the configurations for the build process. (optional) 

Congratulations, your Makefiles were generated!
From now on, 'make' automatically checks whether the CMakeLists.txt
have changed and reconfigures itself accordingly.


Now you could just type

$ make
    to reconfigure the Makefiles and build the project.

-----------------------------------------------------
Quickstart 2: Out-of-source Build
-----------------------------------------------------

cmake is designed so that the build process can be done in a separate
directory. This is highly recommended.

For example, assume that you are in your cvs working directory.
Make sure to remove the file CMakeCache.txt from that directory.

To build kmymoney in the subdirectory ./build/ type

$ mkdir build
$ cd build
$ cmake ..

The directory given to cmake as the main argument is, in this case,
the path to the root source directory, that is, the directory where
the top-level CMakeLists.txt file is stored.

Now you could just type

$ make
    to reconfigure the Makefiles and build the project out-of-source.


Congratulations, you will never have a chaos of generated files
between the important source files again!

-----------------------------------------------------
Quickstart 3: How to compile Debug-Builds
-----------------------------------------------------

As an example configuration option, you would like to configure a
debug build just as './configure --enable-debug=full' did before.

For this, assuming you are in ./build/, you can either use a gui

$ ccmake .
    and change the option CMAKE_BUILD_TYPE to 'Debug'.  Selecting an
    option and pressing 'h' will show you its allowed values.

Or you can pass the option to cmake on the command line

$ cmake -D CMAKE_BUILD_TYPE=Debug .

In any case your choices are safely stored in the file CMakeCache.txt
which will never be completely overwritten

-----------------------------------------------------
Quickstart 4: More options
-----------------------------------------------------

-D CMAKE_INSTALL_PREFIX=<path_to_install_dir>
    This option tells cmake where to install kmymoney to.
    During development, this should be a directory in your development
    environment, such that you can debug the program and test the
    installation.
    The default is "/usr/local/".

-D CMAKE_BUILD_TYPE=<type>
    Choose the type of build, possible values are:
    None (CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used)
    Debug Release RelWithDebInfo MinSizeRel.

-D KDE3_BUILD_TESTS=ON
    To also build the tests.

-----------------------------------------------------
Quickstart 5: Makefile targets
-----------------------------------------------------

After cmake has finished, you have a set of ordinary makefiles in your
directory.  You can type

$ make help
    to see all the targets that are defined.

$ make
    to reconfigure the Makefiles and build the project.

$ make install
    to build and install kmymoney to the previously given install
    directory at once.
    You find the executable in CMAKE_INSTALL_PREFIX/bin

$ make package
    to create some release packages

$ make package_source
    to create a source package
    (Warning: have a clean source directory and build out-of-source)

$ make messages
    to extract and merge translations. This will change the source directory.

$ make message-stats
    to create the translation status for every language.

$ make developer-doc
    to create the developer handbook. Will also create html-version.

$ make test
    to process all unit tests.

-----------------------------------------------------
How to create release packages
-----------------------------------------------------

Best use CPack for release generation.
'make package' automatically invokes CPack with its default options.
By default, only .tar.gz and .tar.bz2 packages are built.

To create an rpm, say, go to the build directory and just type

$ cpack -G RPM

You might need to set distribution-specific options, though.

For general info on cpack, please read
http://www.cmake.org/Wiki/CMake:Packaging_With_CPack

For distribution-specific information, please read
http://www.cmake.org/Wiki/CMake:CPackPackageGenerators






-----------------------------------------------------
What is not working?
-----------------------------------------------------

(Joerg) I'm still working on the message extracting and merging. 
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


