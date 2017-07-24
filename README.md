# Pololu USB AVR Programmer v2 software

Version: 1.0.2<br/>
Release date: 2016 May 05<br/>
[www.pololu.com](https://www.pololu.com/)

This repository contains the source code of the configuration software that
supports the [Pololu USB AVR Programmer v2](https://www.pololu.com/product/3170).
There are two programs: the Pololu USB AVR Programmer v2 Command-line Utility
(pavr2cmd) and the Pololu USB AVR Programmer v2 Configuration Utility (pavr2gui).

This software only supports the Pololu USB AVR Programmer *v2*, which is
*blue-colored* and labeled *pgm04a*.  If you have an older Pololu programmer,
refer to the product page and user's guide of that programmer for software
resources.

Note that the *v2* in the name of this repository refers to the version of the
hardware product, which is different from the version number of this software.

This software is only used for configuring the programmer and reading
information from it.  To actually program an AVR, you will need AVR programming
software such as AVRDUDE or Atmel Studio.

## Installation

Installers for Windows and Mac OS X are available for download from the Pololu
USB AVR Programmer v2 User's Guide:

  http://www.pololu.com/docs/0J67

Linux users can download the source code and compile it.  See
[BUILDING.md](BUILDING.md) for instructions.

## Version history

* 1.0.2 (2016 May 05): Fixed a problem with the Mac OS X release that prevented
  it from finding libusbp at run-time.
* 1.0.1 (2016 Mar 20):
    * Fixed the Windows installer so that it sends the appropriate message to
      notify other applications that the PATH has changed.
    * Changed the Windows installer "repair" option so that it reinstalls
      the driver files when it is run.
* 1.0.0 (2016 Mar 02): Original release.
