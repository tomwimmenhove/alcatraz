DEPDIR := .d
$(shell mkdir -p $(DEPDIR) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td
POSTCOMPILE = @mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

INCLUDEDIRS = -I../../rpcbuf/src/ -I../../src/guest/
INCLUDENEWLIB  = -I ./newlib/newlib/libc/include/
DEBUGFLAGS = -ggdb -DDEBUG -fstack-protector-all
CPPFLAGS = $(DEBUGFLAGS) $(INCLUDEDIRS) -O2 -std=c++17 -fno-threadsafe-statics -fstack-protector-all -nostdlib -ffreestanding -lgcc -mcmodel=kernel -fno-pic -ffunction-sections -fdata-sections -Wl,gc-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit $(DEPFLAGS)
LDFLAGS = -T ../../src/guest/linker.ld

CC = gcc
CPP = g++
AS = gcc # Handles defines through the cpp pre-processor, gas does not.
LD = ld
AR = ar
OBJCOPY = objcopy

OBJS = guest.o alloc.o
SRCS = guest.cpp alloc.cpp

all: payload.o

alloc.o: alloc.cpp
	$(CPP) $(CPPFLAGS) $(INCLUDENEWLIB) -c $<
	$(POSTCOMPILE)

%.o: %.cpp
	$(CPP) $(CPPFLAGS) -c $<
	$(POSTCOMPILE)

guest.elf: guest.o alloc.o ../../src/guest/libguest.a
	$(LD) $(LDFLAGS) guest.o alloc.o -o $@ -L ../../src/guest/ -l guest newlib/build/libc/libc.a

../../src/guest/libguest.a:
	make -C ../../src/guest/

guest.img: guest.elf
	# XXX: Is the section flag correct?
	$(OBJCOPY) -O binary --set-section-flags '.bss*'=alloc,load,contents $< $@

guest.img.o: guest.img
	$(LD) -b binary -r $< -o $@

payload.o: guest.img payload.ld guest.img.o
	$(LD) -T payload.ld -o payload.o

clean:
	rm -f *.elf *.img *.o *.a

$(DEPDIR)/%.d: ;
	.PRECIOUS: $(DEPDIR)/%.d

include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS))))

