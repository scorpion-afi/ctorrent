#!/bin/bash

# this script is supposed to be called from CONTAINER !!!
# do NOT call it directly from host !!!

# Note: containers have to set up at least the next environment variable:
#       CROSS_TRIPLE  - cross triple (e.g. arm-linux-androideabi)
#       CMAKE_TOOLCHAIN_FILE - toolchain file for cross-compilation with cmake


if [ x${CROSS_TRIPLE} == x ]; then
    echo "CROSS_TRIPLE env variable isn't set."
    exit
fi

install_dir="BUILD_ROOT"
pkg_name="ctorrent-${CROSS_TRIPLE}.tar.gz"

# it's supposed that we're in /build directory

rm -rf ${CROSS_TRIPLE}
mkdir ${CROSS_TRIPLE}
cd ${CROSS_TRIPLE}

# create a directory to be root during the install phase
mkdir ${install_dir}

echo "============================================================================================"
echo "Get started to build for ${CROSS_TRIPLE}..."
echo ""

# within the container a CMAKE_TOOLCHAIN_FILE env variable points to the
# toolchain.cmake file which instructs cmake where to look for compilers,
# linkers..., includes, libraries... and so on
#
# /project/src - is where CMakeLists.txt placed.
cmake -DCMAKE_TOOLCHAIN_FILE=$CMAKE_TOOLCHAIN_FILE /project/src
make
make DESTDIR=${install_dir} install

if [ $? == 0 ]; then
  cd ${install_dir} && tar -czf ../../${pkg_name} * && cd ..
  echo -e "\nBuild for ${CROSS_TRIPLE} has been successfully finished."
  echo -e "You may find a result package ${pkg_name} within the ./build directory.\n"
fi
