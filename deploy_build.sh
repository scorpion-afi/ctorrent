#!/bin/bash

# this script deploys bulding processes within different
# containers to build for different targets (OS, architectures)

# Note: containers have to set up at least the next environment variable:
#       CROSS_TRIPLE  - cross triple (e.g. arm-linux-androideabi)
#       CMAKE_TOOLCHAIN_FILE - toolchain file for cross-compilation with cmake

containers="thewtex/cross-compiler-linux-x64
            thewtex/cross-compiler-android-arm"

mkdir -p build

for container_name in ${containers}; do
  sudo docker run --rm \
   -v ${PWD}:/project:ro \
   -v ${PWD}/build:/build:rw \
   -w /build \
   ${container_name} \
   bash /project/build_target.sh
done