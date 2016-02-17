#!/usr/bin/env ruby

# This is a script for Mac OS X that fixes the shared library paths embedded in
# an executable to make the executable suitable for public release.  It also
# copies each referenced library into the same directory as the executable.

require 'fileutils'

# Reads the shared library specifications from an executable.
# These are generally paths to libraries, but some of them start with
# special symbols like @rpath or @executable_path.
def get_lib_paths(filename)
  otool_lines = `otool -L '#{filename}'`.split("\n")
  lib_paths = []
  otool_lines.each do |line|
    next if !line.start_with?("\t")
    lib_paths << line.split(' ')[0].strip
  end
  lib_paths
end

# Changes the reference to a library inside an executable.
def fix_ref(exe_filename, old_ref, new_ref)
  success = system "install_name_tool -change '#{old_ref}' '#{new_ref}' '#{exe_filename}'"
  exit 3 if !success
end

# Copies a library into the specified directory and normalizes its
# permissions.
def copy_lib(lib, dir)
  FileUtils.cp lib, dir, verbose: true

  # Normalize the permissions because Homebrew libraries seem
  # seem to have 0444 permissions, which is annoying.
  FileUtils.chmod 0755, "#{dir}/#{File.basename lib}"
end

if ARGV.size != 1
  $stderr.puts "Usage: fix_dylibs.rb EXECUTABLE_FILENAME"
  exit 1
end

failure = false

exe_filename = ARGV[0]
exe_dirname = File.dirname(exe_filename)
get_lib_paths(exe_filename).each do |lib_path|
  basename = File.basename(lib_path)

  case
  when lib_path.start_with?("/usr/lib")
    # This is a system library that we don't need to worry about.
  when lib_path.start_with?("@rpath/")
    # This library uses @rpath.  It is probably in /usr/local/lib.
    # Just copy the library.
    copy_lib "/usr/local/lib/#{basename}", exe_dirname
  when lib_path.start_with?("@executable_path/")
    # This library is probably in the right place already.
    # If not, we'll catch the problem when we test the executable.
  else
    # Copy the library and fix the reference to it.
    copy_lib lib_path, exe_dirname
    fix_ref(exe_filename, lib_path, "@rpath/#{basename}")
  end
end



exit 2 if failure
