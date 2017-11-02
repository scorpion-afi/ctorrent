#!/bin/bash

# this script got to be executed inside container launched by 'launch_container.sh'.

tar -C / -xvf /home/dev/project/build/ctorrent-x86_64-linux.tar.gz

# the package may contain some libraries, so we have to inform ld
ldconfig