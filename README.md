# Pololu USB AVR Programmer v2 software

Version: 1.1.0<br/>
Release date: 2018-03-23<br/>
[www.pololu.com](https://www.pololu.com/)

This repository contains the source code of the configuration software that
supports the [Pololu USB AVR Programmer v2](https://www.pololu.com/product/3170)
and [Pololu USB AVR Programmer v2.1](https://www.pololu.com/product/3172).
There are two programs: the Pololu USB AVR Programmer v2 Command-line Utility
(pavr2cmd) and the Pololu USB AVR Programmer v2 Configuration Utility (pavr2gui).

This software only supports the Pololu USB AVR Programmer *v2* and *v2.1*,
which are *blue-colored*.  If you have an older Pololu programmer,
refer to the product page and user's guide of that programmer for software
resources.

Note that the *v2* in the name of this repository refers to the versions of the
hardware products, which is different from the version number of this software.

This software is only used for configuring the programmer and reading
information from it.  To actually program an AVR, you will need AVR programming
software such as AVRDUDE or Atmel Studio.

## Installation

Installers for this software are available for download from the
[Pololu USB AVR Programmer v2 User's Guide][guide].

See [BUILDING.md](BUILDING.md) for information about how to compile
the software from source.

## Version history

* 1.1.1 (2024-12-23):
    * No changes to this software, but having this new version number helps
      us release an updated version that works on the Raspberry Pi 5.
* 1.1.0 (2018-03-23):
    * Added support for the Pololu USB AVR Programmer v2.1
    * Changed the installers to be built with nixcrpkgs.
    * GUI: Center the window at startup.
* 1.0.2 (2016-05-05): Fixed a problem with the macOS release that prevented
  it from finding libusbp at run-time.
* 1.0.1 (2016-03-20):
    * Fixed the Windows installer so that it sends the appropriate message to
      notify other applications that the PATH has changed.
    * Changed the Windows installer "repair" option so that it reinstalls
      the driver files when it is run.
* 1.0.0 (2016-03-02): Original release.

[guide]: http://www.pololu.com/docs/0J67
