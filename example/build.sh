#!/bin/bash

g++ -I ../src/host/ -I ../kvmpp/src/ -I ../rpcbuf/src/  -std=c++17 -ggdb -o example guest/payload.o example.cpp ../src/host/alcatraz.cpp ../kvmpp/src/kvmpp.cpp  && ./example
