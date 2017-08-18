#!/bin/bash

root_dir=${PWD}/../../

# seccomp=unconfined - 'cause we have to debug
# we have to map 'root_dir' to '/home/dev/project' 'cause the
# same path was used when binaries were built, so to use gdb
# we have to do this
sudo docker run \
--security-opt seccomp=unconfined \
--rm -it \
-v ${root_dir}:/home/dev/project:ro \
-w /home/dev/ \
x86_64-linux-root