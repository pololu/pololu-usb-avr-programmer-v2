require 'fileutils'
require 'pathname'
include FileUtils

ENV['PATH'] = ENV.fetch('_PATH')

ConfigName = ENV.fetch('config_name')
OutDir = Pathname(ENV.fetch('out'))
PayloadDir = Pathname(ENV.fetch('payload'))
SrcDir = Pathname(ENV.fetch('src'))
Version = File.read(PayloadDir + 'version.txt')

StagingDir = Pathname('pavr2-macos-files')
OutTar = OutDir + "#{StagingDir}.tar"
AppName = 'Pololu USB AVR Programmer v2'
PkgFile = "pololu-usb-avr-programmer-v2-#{Version}-#{ConfigName}.pkg"
PkgId = 'com.pololu.pavr2'
AppExe = 'pavr2gui'
CliExe = 'pavr2cmd'

ReleaseDir = Pathname('release')
AppDir = ReleaseDir + "#{AppName}.app"
ContentsDir = AppDir + 'Contents'
BinDir = ContentsDir + 'MacOS'
AppResDir = ContentsDir + 'Resources'
PathDir = Pathname('path')
ResDir = Pathname('resources')

mkdir_p StagingDir
cd StagingDir
mkdir_p BinDir
mkdir_p AppResDir
mkdir_p PathDir
mkdir_p ResDir

cp_r Dir.glob(PayloadDir + 'bin' + '*'), BinDir
cp ENV.fetch('license'), ContentsDir + 'LICENSE.html'

File.open(ContentsDir + 'Info.plist', 'w') do |f|
  f.puts <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleDevelopmentRegion</key>
  <string>en</string>
  <key>CFBundleExecutable</key>
  <string>#{AppExe}</string>
  <key>CFBundleIconFile</key>
  <string>app.icns</string>
  <key>CFBundleIdentifier</key>
  <string>#{PkgId}.app</string>
  <key>CFBundleInfoDictionaryVersion</key>
  <string>6.0</string>
  <key>CFBundleName</key>
  <string>#{AppName}</string>
  <key>CFBundlePackageType</key>
  <string>APPL</string>
  <key>CFBundleShortVersionString</key>
  <string>#{Version}</string>
  <key>CFBundleSignature</key>
  <string>????</string>
  <key>CFBundleVersion</key>
  <string>#{Version}</string>
  <key>NSHumanReadableCopyright</key>
  <string>Copyright (C) #{Time.now.year} Pololu Corporation</string>
</dict>
</plist>
EOF
end

cp SrcDir + 'images' + 'app.icns', AppResDir

File.open(PathDir + '99-pololu-avr2', 'w') do |f|
  f.puts "/Applications/#{AppName}.app/Contents/MacOS"
end

File.open(ResDir + 'welcome.html', 'w') do |f|
  f.puts <<EOF
<!DOCTYPE html>
<html>
<style>
body { font-family: sans-serif; }
</style>
<title>Welcome</title>
<p>
This package installs the configuration software for the
<b>Pololu USB AVR Programmer v2</b> on your computer.
Please note that <b>v2</b> refers to the versions of the
hardware products.
The version number of this software is #{Version}.
<p>
This software only supports the Pololu USB AVR Programmer <b>v2</b> and <b>v2.1</b>,
which are <b>blue-colored</b>.  If you have an older Pololu
programmer, refer to the product page and user's guide of that programmer for
software.
<p>
This package will install two programs:
<ul>
<li>Pololu USB AVR Programmer v2 Configuration Utility (pavr2gui)
<li>Pololu USB AVR Programmer v2 Command-line Utility (pavr2cmd)
</ul>
<br>
<p>
This software can only configure the programmer or read information from it.
To actually program an AVR, you will need AVR programming software such as
AVRDUDE or the Arduino IDE.
EOF
end

File.open('distribution.xml', 'w') do |f|
  f.puts <<EOF
<?xml version="1.0" encoding="utf-8" standalone="no"?>
<installer-gui-script minSpecVersion="2">
  <title>#{AppName} #{Version}</title>
  <welcome file="welcome.html" />
  <pkg-ref id="app">app.pkg</pkg-ref>
  <pkg-ref id="path">path.pkg</pkg-ref>
  <options customize="allow" require-scripts="false" />
  <domain enable_anywhere="false" enable_currentUserHome="false"
    enable_localSystem="true" />
  <choices-outline>
    <line choice="app" />
    <line choice="path" />
  </choices-outline>
  <choice id="app" visible="false">
    <pkg-ref id="app" />
  </choice>
  <choice id="path" title="Add binary directory to the PATH"
    description="Adds an entry to /etc/paths.d/ so you can easily run #{CliExe} from a terminal.">
    <pkg-ref id="path" />
  </choice>
  <volume-check>
    <allowed-os-versions>
      <os-version min="10.11" />
    </allowed-os-versions>
  </volume-check>
</installer-gui-script>
EOF
end

File.open('build.sh', 'w') do |f|
  f.puts <<EOF
set -ue

pkgbuild --analyze --root zzz nocomponents.plist

pkgbuild \\
  --identifier #{PkgId}.app \\
  --version "#{Version}" \\
  --root "#{ReleaseDir}" \\
  --install-location /Applications \\
  --component-plist nocomponents.plist \\
 app.pkg

pkgbuild \\
  --identifier #{PkgId}.path \\
  --version "#{Version}" \\
  --root "#{PathDir}" \\
  --install-location /etc/paths.d \\
  --component-plist nocomponents.plist \\
  path.pkg

productbuild \\
  --identifier #{PkgId} \\
  --version "#{Version}" \\
  --resources "#{ResDir}" \\
  --distribution distribution.xml \\
  "#{PkgFile}"
EOF
end

mkdir_p OutDir
chmod_R 'u+w', '.'
chmod 'u+x', 'build.sh'
cd '..'
success = system("tar cfv #{OutTar} #{StagingDir}")
raise "tar failed: error #{$?.exitstatus}" if !success
