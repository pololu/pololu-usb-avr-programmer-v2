#!/bin/bash

# This is an MSYS2 bash script that we use at Pololu to generate the official
# installer for Windows.  To use it:
#
# 1. Copy app.ico, setup_banner_wix.bmp, and setup_welcome_wix.bmp to the
#    images directory.  We don't include these files in source
#    control because they contain the Pololu logo.  Note:
#    - setup_banner_wix.bmp should be 493x58
#    - setup_welcome_wix.bmp should be 493x312
# 2. From your CMake build directory, run:
#      make && sh ../windows-installer/release.sh

MSBUILD="/c/Program Files (x86)/MSBuild/14.0/Bin/MSBuild.exe"
SIGNTOOL="/c/Program Files (x86)/Windows Kits/10/bin/x64/signtool.exe"
DLLDIR="msi_dlls"
EXEDIR="msi_exes"
SIGNFLAGS="-fd sha256 -tr http://timestamp.globalsign.com/?signature=sha2 -td sha256"

if [ "$MSYSTEM" == "MINGW32" ]; then
    MINGW_PREFIX="/mingw32"
else
    # We don't make installers for 64-bit software.
    echo "Invalid value for MSYSTEM: $MSYSTEM"
    exit 1
fi

set -ue

rm -rf "$DLLDIR" "$EXEDIR"
mkdir -p "$DLLDIR/platforms" "$EXEDIR"

# These are the files you need for the CLI.
cp "$MINGW_PREFIX/bin/libwinpthread-1.dll" "$DLLDIR" # MIT license + custom
cp "$MINGW_PREFIX/bin/libstdc++-6.dll" "$DLLDIR"    # GPLv3+
cp "$MINGW_PREFIX/bin/libgcc_s_dw2-1.dll" "$DLLDIR" # GPLv3+
cp "$MINGW_PREFIX/bin/libusbp-1.dll" "$DLLDIR"      # our MIT license
cp "pavr2cmd.exe" "$EXEDIR"

# These are the additional files needed for the GUI.
cp "$MINGW_PREFIX/bin/libbz2-1.dll" "$DLLDIR"       # custom license
cp "$MINGW_PREFIX/bin/libfreetype-6.dll" "$DLLDIR"  # GPLv2+
cp "$MINGW_PREFIX/bin/libglib-2.0-0.dll" "$DLLDIR"  # LGPLv2+
cp "$MINGW_PREFIX/bin/libharfbuzz-0.dll" "$DLLDIR"  # "Old MIT"
cp "$MINGW_PREFIX/bin/libiconv-2.dll" "$DLLDIR"     # GPLv3
cp "$MINGW_PREFIX/bin/libintl-8.dll" "$DLLDIR"      # GPLv3
cp "$MINGW_PREFIX/bin/libpcre16-0.dll" "$DLLDIR"    # BSD
cp "$MINGW_PREFIX/bin/libpng16-16.dll" "$DLLDIR"    # custom
cp "$MINGW_PREFIX/bin/Qt5Core.dll" "$DLLDIR"        # LGPLv3
cp "$MINGW_PREFIX/bin/Qt5Gui.dll" "$DLLDIR"
cp "$MINGW_PREFIX/bin/Qt5Widgets.dll" "$DLLDIR"
cp "$MINGW_PREFIX/share/qt5/plugins/platforms/qwindows.dll" "$DLLDIR/platforms"
cp "$MINGW_PREFIX/bin/zlib1.dll" "$DLLDIR"          # custom
cp "pavr2gui.exe" "$EXEDIR"

# If we build with a no-ICU version of Qt5, we don't need these:
# cp "$MINGW_PREFIX/bin/libicudt56.dll" "$DLLDIR"  # custom
# cp "$MINGW_PREFIX/bin/libicuin56.dll" "$DLLDIR"  # (same)
# cp "$MINGW_PREFIX/bin/libicuuc56.dll" "$DLLDIR"  # (same)

"$SIGNTOOL" sign -n "Pololu Corporation" $SIGNFLAGS "$EXEDIR/*.exe"

"$MSBUILD" -t:rebuild -p:Configuration=Release -p:TreatWarningsAsErrors=True \
  windows-installer/pavr2.wixproj

cp windows-installer/bin/Release/en-us/*.msi .

"$SIGNTOOL" sign -n "Pololu Corporation" $SIGNFLAGS -d "Pololu USB AVR Programmer v2 Setup" *.msi
