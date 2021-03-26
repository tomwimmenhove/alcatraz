#!/bin/bash

#if 1
CRTBEGIN_OBJ=`g++ -print-file-name=crtbegin.o`
CRTEND_OBJ=`g++ -print-file-name=crtend.o`
#else
CRTBEGIN_OBJ=crt/crtbegin.o
CRTEND_OBJ=crt/crtend.o
#endif

# Compile the startup and guest code

gcc  -c crti.S -o crti.o
gcc  -c crtn.S -o crtn.o

g++ -std=c++17 -fno-threadsafe-statics  -ggdb -DDEBUG -fstack-protector-all -O2  -nostdlib -ffreestanding -lgcc -mcmodel=kernel -fno-pic -ffunction-sections -fdata-sections -Wl,gc-sections -fno-exceptions -fno-rtti -c -o guest64.o guest.cpp || exit

g++ -std=c++17 -fno-threadsafe-statics  -ggdb -DDEBUG -fstack-protector-all -O2  -nostdlib -ffreestanding -lgcc -mcmodel=kernel -fno-pic -ffunction-sections -fdata-sections -Wl,gc-sections -fno-exceptions -fno-rtti -c -o klib.o klib.cpp || exit

# Compile into an elf (easier for disassembly and debugging */
ld -T linker.ld crti.o $CRTBEGIN_OBJ guest64.o klib.o $CRTEND_OBJ crtn.o -o guest64.elf

# Create a raw binary
objcopy -O binary guest64.elf guest64.img

# Create a payload with the binary in it
ld -b binary -r guest64.img -o guest64.img.o
ld -T payload.ld -o payload.o

# Finally, compile the application
g++ -std=c++17 -ggdb -o example payload.o example.cpp kvmpp/src/kvmpp.cpp  && ./example 
