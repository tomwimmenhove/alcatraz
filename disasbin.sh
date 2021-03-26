#!/bin/bash

objdump -D -Matt,x86-64 -b binary -m i386 "$1" | less


