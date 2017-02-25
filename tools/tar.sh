#!/bin/bash

#TARGET=gcc-arm-none-eabi-5_4-2016q3
TARGET=arm-2011.03

#tar
#tar -czf $TARGET.tar.gz $TARGET
#split -b 80M -d -a 1 $TARGET.tar.gz $TARGET.tar.gz.

#untar
cat $TARGET.tar.gz.* | tar -zxv

rm $TARGET.tar.gz

