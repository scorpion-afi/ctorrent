#!/bin/bash

# this script is supposed to be called from CONTAINER !!!
# do NOT call it directly from the host !!!

# Note: containers have to set up at least the next environment variables:
#       TARGET_NAME  - name of a target (e.g. arm-android)
#       CMAKE_TOOLCHAIN_FILE - a toolchain file for cross-compilation with cmake,
#                              if any cross-compilation is supposed


if [ x${TARGET_NAME} == x ]; then
    echo "TARGET_NAME env variable isn't set."
    exit
fi

install_dir="BUILD_ROOT"
pkg_name="ctorrent-${TARGET_NAME}.tar.gz"

# it's supposed that we're in /home/dev/build directory

rm -f build_success
#rm -rf ${TARGET_NAME}          # TODO: a flag to make a clean rebuild should be provided  
mkdir -p ${TARGET_NAME}
cd ${TARGET_NAME}

# create a directory to be root during the install phase
mkdir -p ${install_dir}

echo "============================================================================================"
echo "Get started to build for ${TARGET_NAME}..."
echo ""

# TODO: think how to pass options to cmake outside the container
# within the container a CMAKE_TOOLCHAIN_FILE env variable points to the
# toolchain.cmake file which instructs cmake where to look for compilers,
# linkers..., includes, libraries... and so on
#
# /home/dev/project/src - is where CMakeLists.txt placed.
cmake -DCMAKE_TOOLCHAIN_FILE=$CMAKE_TOOLCHAIN_FILE -DDEBUG=ON -DTESTS=ON /home/dev/project/src
#make VERBOSE=1
make
make DESTDIR=${install_dir} install

if [ $? == 0 ]; then
  cd ${install_dir} && tar -czvf ../../${pkg_name} * && cd ..
  echo -e "\nBuild for ${TARGET_NAME} has been successfully finished."
  echo -e "You may find a result package ${pkg_name} within the ./build directory.\n"
  
  # an insane way to expose outside that the build was successful
  touch ../build_success
fi
