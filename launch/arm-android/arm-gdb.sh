#!/bin/sh

if [ x${1} = x ]; then
  echo "specify a binary name to debug."
  echo "example:"
  echo " $ arm-gdb app_to_debug [arguments to app]"
  exit 1
fi

binaries_path=../../build/arm-android/BUILD_ROOT/system/bin

arm-linux-androideabi-gdb -x gdb_cmd.txt --args ${binaries_path}/${1} ${2}  
