#!/bin/bash

make -C ../../src/guest/

g++ -I../../rpcbuf/src/ -std=c++17 -fno-threadsafe-statics  -ggdb -DDEBUG -fstack-protector-all -O2  -nostdlib -ffreestanding -lgcc -mcmodel=kernel -fno-pic -ffunction-sections -fdata-sections -Wl,gc-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -c -o guest64.o guest.cpp || exit

ld -T ../../src/guest/linker.ld  guest64.o  -o guest64.elf -L ../../src/guest/ -l guest 

# Create a raw binary
objcopy -O binary guest64.elf guest64.img

# Create a payload with the binary in it
ld -b binary -r guest64.img -o guest64.img.o
ld -T payload.ld -o payload.o

