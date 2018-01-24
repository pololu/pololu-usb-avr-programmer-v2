# Building from source

If you want to build this software from its source code, you can follow these
instructions.  However, for most Windows and Mac OS X users, we recommend
installing this software using our pre-built installers and not trying to build
from source.


## Building from source on Linux

You will need to install tar, wget, gcc, make, CMake, libudev, and Qt 5.  Most
Linux distributions come with a package manager that you can use to install
these dependencies.

On Ubuntu, Raspbian, and other Debian-based distributions, you can install the
dependencies by running:

    sudo apt-get install build-essential git cmake libudev-dev qtbase5-dev

On Arch Linux, libudev should already be installed, and you can install the
other dependencies by running:

    pacman -Sy base-devel git cmake qt5-base

For other Linux distributions, consult the documentation of your distribution
for information about how to install these dependencies.

This software depends on the Pololu USB Library version 1.x.x (libusbp-1).  Run
the commands below to download, compile, and install the latest version of that
library on your computer.

    git clone https://github.com/pololu/libusbp -b v1-latest
    cd libusbp
    mkdir build
    cd build
    cmake ..
    make
    sudo make install
    cd ../..

You can test to see if libusbp-1 was installed correctly by running
`pkg-config libusbp-1 --cflags`,
which should output something like
`-I/usr/local/include/libusbp-1`.
If it says that libusbp-1 was not found in the pkg-config search path,
retry the instructions above.

Run these commands to download, build, and install this software:

    git clone https://github.com/pololu/pololu-usb-avr-programmer-v2 pavr2
    cd pavr2
    mkdir build
    cd build
    cmake ..
    make
    sudo make install
    cd ../..

You will need to install a udev rule to give non-root users permission to access
Pololu USB devices. Run this command:

    sudo cp udev-rules/99-pololu.rules /etc/udev/rules.d/

You should now be able to run the command-line utility by running `pavr2cmd` in
your shell, and you should be able to start the graphical configuration utility
by running `pavr2gui`.

If you get an error about libusbp failing to load (for example,
"cannot open shared object file: No such file or directory"), then
run `sudo ldconfig` and try again.  If that does not work, it is likely that
your system does not search for libraries in `/usr/local/lib`
by default.  To fix this issue for your current shell session, run:

    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

A more permanent solution, which will affect all users of the computer, is to
run:

    sudo sh -c 'echo /usr/local/lib > /etc/ld.so.conf.d/local.conf'
    sudo ldconfig


## Building from source on Windows with MSYS2

The recommended way to install this software on Windows is to download our
pre-built installer.  However, you can build it from source using
[MSYS2](http://msys2.github.io/).  If you have not done so already, follow the
instructions on the MSYS2 website to download, install, and update your MSYS2
environment.  In particular, be sure to update your installed packages.

Next, start a shell by selecting "MinGW-w64 Win32 Shell" from your Start menu or
by running `mingw32.exe`.  This is the right environment to use if you want to
build 32-bit software that works on 32-bit or 64-bit Windows.  (If you want to
build 64-bit software that only works on 64-bit versions of Windows, select
"MinGW-w64 Win64 Shell" or `mingw64.exe`.)

Run this command to install the required development tools:

    pacman -S base-devel git $MINGW_PACKAGE_PREFIX-{toolchain,cmake,tinyxml2,qt5}

If pacman prompts you to enter a selection of packages to install, just press
enter to install all of the packages.

This software depends on the Pololu USB Library version 1.x.x (libusbp-1).  Run
the commands below to download, compile, and install the latest version of that
library into your MSYS2 environment.

    git clone https://github.com/pololu/libusbp -b v1-latest
    cd libusbp
    mkdir build
    cd build
    MSYS2_ARG_CONV_EXCL=- cmake .. -G"MSYS Makefiles" -DCMAKE_INSTALL_PREFIX=$MINGW_PREFIX
    make install DESTDIR=/
    cd ../..

You can test to see if libusbp-1 was installed correctly by running
`pkg-config libusbp-1 --cflags`,
which should output something like
`-IC:/msys64/mingw32/include/libusbp-1`.
If it says that libusbp-1 was not found in the pkg-config search path,
retry the instructions above.

Run these commands to build this software and install it:

    git clone https://github.com/pololu/pololu-usb-avr-programmer-v2 pavr2
    cd pavr2
    mkdir build
    cd build
    MSYS2_ARG_CONV_EXCL=- cmake .. -G"MSYS Makefiles" -DCMAKE_INSTALL_PREFIX=$MINGW_PREFIX
    make install DESTDIR=/
    cd ../..

You should now be able to run the command-line utility by running `pavr2cmd` in
your shell, and you should be able to start the graphical configuration utility
by running `pavr2gui`.


## Building from source on Mac OS X with Homebrew

The recommended way to install this software on Mac OS X is to download our
pre-built installer.  However, you can also build it from source.

First, install [Homebrew](http://brew.sh/).

Then use brew to install the dependencies:

    brew install pkg-config cmake tinyxml2 qt5

This software depends on the Pololu USB Library version 1.x.x (libusbp-1).  Run
the commands below to download, compile, and install the latest version of that
library on your computer.

    git clone https://github.com/pololu/libusbp -b v1-latest
    cd libusbp
    mkdir build
    cd build
    cmake ..
    make
    sudo make install
    cd ../..

You can test to see if libusbp-1 was installed correctly by running
`pkg-config libusbp-1 --cflags`,
which should output something like
`-I/usr/local/include/libusbp-1`.
If it says that libusbp-1 was not found in the pkg-config search path,
retry the instructions above.

Run these commands to build this software and install it:

    git clone https://github.com/pololu/pololu-usb-avr-programmer-v2 pavr2
    cd pavr2
    mkdir build
    cd build
    cmake .. -DCMAKE_PREFIX_PATH=$(brew --prefix qt5)
    make
    sudo make install
    cd ../..

You should now be able to run the command-line utility by running `pavr2cmd` in
your shell, and you should be able to start the graphical configuration utility
by running `pavr2gui`.


## Building from source on Linux for Windows with Nixcrpkgs

The Windows version of this software can also be cross-compiled on a Linux
machine using [Nixcrpkgs](https://github.com/pololu/nixcrpkgs), a collection of tools for
cross-compiling.  One advantage of compiling the software this way is that you get standalone executables that do not depend on any shared libraries except those that come with the operating system.

To get started, you should first install [Nix, the purely functional
package manager](http://nixos.org/nix/), on a Linux machine by following the
instructions on the Nix website.

Next, download the latest version of
[Nixcrpkgs](https://github.com/pololu/nixcrpkgs), a collection of tools for
cross-compiling, and add nixcrpkgs to your `NIX_PATH` environment variable.  In
these instructions, we will download it to your home directory, but you could
put it somewhere else if you want.

    cd ~
    wget https://github.com/pololu/nixcrpkgs/archive/master.tar.gz
    tar -xf master.tar.gz
    mv nixcrpkgs-master nixcrpkgs
    export NIX_PATH=$NIX_PATH:nixcrpkgs=$(pwd)/nixcrpkgs

Now run these commands to download this software and build it:

    wget https://github.com/pololu/pololu-usb-avr-programmer-v2/archive/master.tar.gz
    tar -xzf master.tar.gz
    cd pololu-usb-avr-programmer-v2-master
    nix-build -A win32

At this point, `nix-build` will start running the build procedure defined in
`default.nix`.  The first time you run this command, it will take a while to
complete because `nix-build` has to build the cross-compilers and libraries
defined in Nixcrpkgs that are needed for this software.

Once the build is completed, there should be a symbolic link in the current
directory named `result` that points to the compiled software.
