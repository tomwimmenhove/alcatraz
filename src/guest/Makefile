DEPDIR := .d
$(shell mkdir -p $(DEPDIR) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td
POSTCOMPILE = @mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

INCLUDEDIRS = -I../../rpcbuf/src/ -I../../newlib/newlib/libc/include/
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
	mkdir -p tmp
	cd tmp ; ar -x ../../../newlib/build/libc.a
	$(AR) $(ARFLAGS) $@ tmp/*.o $(OBJS)
	rm -rf tmp

clean:
	rm -rf *.o *.a

$(DEPDIR)/%.d: ;
	.PRECIOUS: $(DEPDIR)/%.d

include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS))))
