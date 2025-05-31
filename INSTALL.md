Build Instructions
==================

HTMLCSS requires a C99 compiler like GCC, Clang, or Visual C, along with
POSIX-compliant `make` and `sh` programs (Linux/Unix), Visual Studio (Windows),
and/or Xcode (macOS) for doing the build.  The only required library is ZLIB
(1.1 or later) for compression support.


Getting Prerequisites
---------------------

Run the following commands on CentOS/Fedora/RHEL:

    sudo dnf groupinstall 'Development Tools'
    sudo dnf install zlib-devel

Run the following commands on Debian/Raspbian/Ubuntu:

    sudo apt-get install build-essential zlib1g-dev

Install Xcode from the AppStore on macOS.

Install Visual Studio 2019 or later on Windows.  The Visual Studio solution
"htmlcss.sln" will download all the prerequisite NuGet packages.


Building HTMLCSS
----------------

HTMLCSS uses the usual `configure` script to generate a `make` file:

    ./configure [options]
    make

Use `./configure --help` to see a full list of options.

There is also an Xcode project under the `xcode` directory that can be used on
macOS:

    open xcode/htmlcss.xcodeproj

and a Visual Studio solution under the `vcnet` directory that must be used on
Windows.

You can test the build by running the HTMLCSS test program:

    make test


Installing HTMLCSS
----------------

Once you have successfully built HTMLCSS, install it using:

    sudo make install

By default everything will be installed under `/usr/local`.  Use the `--prefix`
configure option to override the base installation directory.  Set the
`DESTDIR`, `DSTROOT`, or `RPM_BUILD_ROOT` environment variables to redirect the
installation to a staging area, as is typically done for most software packaging
systems (using one of those environment variables...)
