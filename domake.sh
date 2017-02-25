#!/bin/bash


# export tool-chain env
source ./env.rc


make clean
make OptLIB=3 OptSRC=3 all tshow
#make OptLIB=0 OptSRC=0 all tshow

#ls --sort=time -1 -l
cat main.info_size

# shown that we produced code for “Cortex-M3” and lots of other information too
#arm-none-eabi-readelf -A main.elf

# Use objdump to look at the ELF-file with the -S flag
# arm-none-eabi-objdump -S main.elf

