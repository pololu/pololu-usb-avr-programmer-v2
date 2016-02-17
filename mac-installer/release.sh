#!/bin/bash

# Run this from your CMake build directory to create a Mac OS X flat package
# (installer).

set -ue

if [ -d pavr2gui.app/Contents/Frameworks ]; then
  echo "********************************"
  echo "pavr2gui.app already has frameworks installed.  How did that happen?"
  echo "********************************"
fi

MACDEPLOYQT="$(brew --prefix qt5)/bin/macdeployqt"

VERSION=$(cat version.txt)
STAGINGDIR="mac_release"
RESDIR="mac_resources"
APPDIR="$STAGINGDIR/Pololu USB AVR Programmer v2.app"
PATHDIR="path"
SRCDIR=`dirname $0`/..
PKG="pololu-usb-avr-programmer-v2-$VERSION.pkg"

PATH=$PATH:`dirname $0`  # so we can run other scripts in the same directory

rm -rf "$RESDIR"
mkdir -p "$RESDIR"

cp mac-installer/welcome.html "$RESDIR"

rm -rf "$STAGINGDIR"
mkdir -p "$APPDIR/Contents/"{MacOS,Resources}
cp pavr2gui pavr2cmd "$APPDIR/Contents/MacOS/"
cp mac-installer/Info.plist "$APPDIR/Contents/"
cp "$SRCDIR/images/app.icns" "$APPDIR/Contents/Resources/"
cp "$SRCDIR/LICENSE.html" "$APPDIR/Contents/"
"$MACDEPLOYQT" "$APPDIR"

# We only actually need to call this to copy in libusbp.dylib.
# It copies the library in twice because both executables use it.
fix_dylibs.rb "$APPDIR/Contents/MacOS/pavr2cmd"
fix_dylibs.rb "$APPDIR/Contents/MacOS/pavr2gui"

rm -rf "$PATHDIR"
mkdir -p "$PATHDIR"
echo "/Applications/Pololu USB AVR Programmer v2.app/Contents/MacOS/" > "$PATHDIR/99-pololu-avr2"

pkgbuild --analyze --root zzz nocomponents.plist

pkgbuild \
  --identifier com.pololu.pavr2.app \
  --version "$VERSION" \
  --root "$STAGINGDIR" \
  --install-location /Applications \
  --component-plist nocomponents.plist \
  app.pkg

pkgbuild \
  --identifier com.pololu.pavr2.path \
  --version "$VERSION" \
  --root "$PATHDIR" \
  --install-location /etc/paths.d \
  --component-plist nocomponents.plist \
  path.pkg

productbuild \
  --identifier com.pololu.pavr2 \
  --version "$VERSION" \
  --resources "$RESDIR" \
  --distribution mac-installer/distribution.xml \
  "$PKG"
