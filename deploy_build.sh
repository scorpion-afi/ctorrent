#!/bin/bash

# this script deploys bulding processes within different
# containers to build for different targets (architectures, OS)

# Note: containers have to set up at least the next environment variables:
#       TARGET_NAME  - name of a target (e.g. arm-android)
#       CMAKE_TOOLCHAIN_FILE - a toolchain file for cross-compilation with cmake,
#                              if any cross-compilation is supposed

# be aware 'arm-android' and 'x86_64-linux' images use the different version of
# compilers and boost libraries are built with difference options !!!

containers="arm-android
            x86_64-linux"

mkdir -p build

for container_name in ${containers}; do
  sudo docker run --rm \
   -v ${PWD}:/home/dev/project:ro \
   -v ${PWD}/build:/home/dev/build:rw \
   -w /home/dev/build \
   ${container_name} \
   bash /home/dev/project/build_target.sh
done