#!/bin/bash

# use this script to initialize target before work with it.

adb root
sleep 2

# mount /system for read/write access
adb remount
sleep 1

# mount / for read/write access
adb shell "mount -o remount,rw /"
#adb shell "chmod 777 /"

# install crystax libc and crystax stl
adb push crystax.tar.gz /
adb shell tar -xvf /crystax.tar.gz
adb shell rm -f /crystax.tar.gz

# install crystax boost libraries
adb push boost.tar.gz /
adb shell tar -xvf /boost.tar.gz
adb shell rm -f /boost.tar.gz

# for debug purposes
adb forward tcp:5039 tcp:5039


# TODO: what's about changing of env variables?

#while read -r line; do
#  adb shell "echo \"export ${line}\" >> /etc/mkshrc"
#done < ./env.file
