DEPDIR := .d
$(shell mkdir -p $(DEPDIR) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td
POSTCOMPILE = @mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

INCLUDEDIRS = -I../../rpcbuf/src/
DEBUGFLAGS = -ggdb -DDEBUG -fstack-protector-all
CPPFLAGS = $(DEBUGFLAGS) $(INCLUDEDIRS) -O2 -std=c++17 -fno-threadsafe-statics -fstack-protector-all -nostdlib -ffreestanding -lgcc -mcmodel=kernel -fno-pic -ffunction-sections -fdata-sections -Wl,gc-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit $(DEPFLAGS)
ARFLAGS =  -rcs

CC = gcc
CPP = g++
AS = gcc # Handles defines through the cpp pre-processor, gas does not.
LD = ld
AR = ar

CRTBEGIN_OBJ = $(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ = $(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)

OBJS = crti.o crtn.o start.o klib.o
SRCS = crti.S crtn.S start.cpp klib.cpp

all: libguest.a

%.o: %.S
	$(AS) -c $< -o $@

%.o: %.cpp
	$(CPP) $(CPPFLAGS) -c $<
	$(POSTCOMPILE)

libguest.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

clean:
	rm -rf *.o *.a

$(DEPDIR)/%.d: ;
	.PRECIOUS: $(DEPDIR)/%.d

include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS))))



# Compile the startup and guest code

#gcc  -c crti.S -o crti.o
#gcc  -c crtn.S -o crtn.o

#g++ -I../../rpcbuf/src/ -std=c++17 -fno-threadsafe-statics  -ggdb -DDEBUG -fstack-protector-all -O2  -nostdlib -ffreestanding -lgcc -mcmodel=kernel -fno-pic -ffunction-sections -fdata-sections -Wl,gc-sections -fno-exceptions -fno-rtti -c -o start.o start.cpp || exit

#g++ -I../../rpcbuf/src/ -std=c++17 -fno-threadsafe-statics  -ggdb -DDEBUG -fstack-protector-all -O2  -nostdlib -ffreestanding -lgcc -mcmodel=kernel -fno-pic -ffunction-sections -fdata-sections -Wl,gc-sections -fno-exceptions -fno-rtti -c -o klib.o klib.cpp || exit

#ar -rcs libguest.ai crti.o $CRTBEGIN_OBJ start.o klib.o $CRTEND_OBJ crtn.o 
