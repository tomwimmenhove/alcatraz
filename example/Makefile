DEPDIR := .d
$(shell mkdir -p $(DEPDIR) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td
POSTCOMPILE = @mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

INCLUDEDIRS = -I ../src/host/ -I ../kvmpp/src/ -I ../rpcbuf/src/
DEBUGFLAGS = -ggdb -DDEBUG
CPPFLAGS = $(DEBUGFLAGS) $(INCLUDEDIRS) -std=c++17 $(DEPFLAGS)

CC = gcc
CPP = g++
AS = gcc # Handles defines through the cpp pre-processor, gas does not.
LD = ld
AR = ar

OBJS = example.o
SRCS = example.cpp

all: makeguest example

makeguest:
	make -C guest

%.o: %.cpp
	$(CPP) $(CPPFLAGS) -c $<
	$(POSTCOMPILE)

example: example.o guest/payload.o ../kvmpp/src/kvmpp.o ../src/host/libhost.a
	$(CPP) example.o ../kvmpp/src/kvmpp.o guest/payload.o -o example -L ../src/host/ -lhost

clean:
	rm -f *.o
	make -C guest clean

$(DEPDIR)/%.d: ;
	.PRECIOUS: $(DEPDIR)/%.d

include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS))))



# Compile the startup and guest code

#gcc  -c crti.S -o crti.o
#gcc  -c crtn.S -o crtn.o

#g++ -I../../rpcbuf/src/ -std=c++17 -fno-threadsafe-statics  -ggdb -DDEBUG -fstack-protector-all -O2  -nostdlib -ffreestanding -lgcc -mcmodel=kernel -fno-pic -ffunction-sections -fdata-sections -Wl,gc-sections -fno-exceptions -fno-rtti -c -o guest_init.o guest_init.cpp || exit

#g++ -I../../rpcbuf/src/ -std=c++17 -fno-threadsafe-statics  -ggdb -DDEBUG -fstack-protector-all -O2  -nostdlib -ffreestanding -lgcc -mcmodel=kernel -fno-pic -ffunction-sections -fdata-sections -Wl,gc-sections -fno-exceptions -fno-rtti -c -o klib.o klib.cpp || exit

#ar -rcs libguest.ai crti.o $CRTBEGIN_OBJ guest_init.o klib.o $CRTEND_OBJ crtn.o 
