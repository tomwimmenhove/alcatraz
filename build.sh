#!/bin/bash

#cc -Wall -Wextra -Werror -O2 -m64 -ffreestanding -fno-pic -c -o guest64.o guest.c

CRTBEGIN_OBJ=`g++ -print-file-name=crtbegin.o`
CRTEND_OBJ=`g++ -print-file-name=crtend.o`

as crti.s -o crti.o
as crtn.s -o crtn.o
#g++ -std=c++17 -nostdlib -Wall -Wextra -Werror -O2 -m64 -ffreestanding -nostdlib -fno-pic -c -o guest64_main.o guest.cpp || exit
#g++ crti.o $CRTBEGIN_OBJ guest64_main.o $CRTEND_OBJ crtn.o -o guest64.o -nostdlib #-lgcc

g++ -std=c++17 -nostdlib -Wall -Wextra -Werror -O2 -m64 -ffreestanding -nostdlib -fno-pic -c -o guest64.o guest.cpp || exit
#ld -T linker.ld crti.o $CRTBEGIN_OBJ guest64.o $CRTEND_OBJ crtn.o -o guest64.img

ld -T linker.ld crti.o /home/tom/grive-Tom.Wimmenhove/Projects/toyos/cross/lib/gcc/x86_64-elf/7.1.0/crtbegin.o /home/tom/grive-Tom.Wimmenhove/Projects/toyos/cross/lib/gcc/x86_64-elf/7.1.0/crtend.o crtn.o guest64.o -o guest64.elf

objcopy -O binary guest64.elf guest64.img

ld -b binary -r guest64.img -o guest64.img.o

ld -T payload.ld -o payload.o

g++ -std=c++17 -o example payload.o example.cpp kvmpp/src/kvmpp.cpp  && ./example 
