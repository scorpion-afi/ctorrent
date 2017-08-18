#!/bin/bash

adb push ../../build/ctorrent-arm-android.tar.gz /
adb shell tar -xvf /ctorrent-arm-android.tar.gz
adb shell rm -f /ctorrent-arm-android.tar.gz