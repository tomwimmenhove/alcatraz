DEPDIR := .d
$(shell mkdir -p $(DEPDIR) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td
POSTCOMPILE = @mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

INCLUDEDIRS = -I../../rpcbuf/src/ -I../../kvmpp/src/
DEBUGFLAGS = -ggdb -DDEBUG
CPPFLAGS = $(DEBUGFLAGS) $(INCLUDEDIRS) -std=c++17 $(DEPFLAGS)
ARFLAGS =  -rcs

CC = gcc
CPP = g++
AS = gcc # Handles defines through the cpp pre-processor, gas does not.
LD = ld
AR = ar

OBJS = alcatraz.o kvmpp.o
SRCS = alcatraz.cpp ../../kvmpp/src/kvmpp.cpp

all: libhost.a

kvmpp.o: ../../kvmpp/src/kvmpp.cpp
	$(CPP) $(CPPFLAGS) -c $<
	$(POSTCOMPILE)

%.o: %.cpp
	$(CPP) $(CPPFLAGS) -c $<
	$(POSTCOMPILE)

libhost.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

clean:
	rm -rf *.o *.a

$(DEPDIR)/%.d: ;
	.PRECIOUS: $(DEPDIR)/%.d

include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS))))



# Compile the startup and guest code

#gcc  -c crti.S -o crti.o
#gcc  -c crtn.S -o crtn.o

#g++ -I../../rpcbuf/src/ -std=c++17 -fno-threadsafe-statics  -ggdb -DDEBUG -fstack-protector-all -O2  -nostdlib -ffreestanding -lgcc -mcmodel=kernel -fno-pic -ffunction-sections -fdata-sections -Wl,gc-sections -fno-exceptions -fno-rtti -c -o guest_init.o guest_init.cpp || exit

#g++ -I../../rpcbuf/src/ -std=c++17 -fno-threadsafe-statics  -ggdb -DDEBUG -fstack-protector-all -O2  -nostdlib -ffreestanding -lgcc -mcmodel=kernel -fno-pic -ffunction-sections -fdata-sections -Wl,gc-sections -fno-exceptions -fno-rtti -c -o klib.o klib.cpp || exit

#ar -rcs libguest.ai crti.o $CRTBEGIN_OBJ guest_init.o klib.o $CRTEND_OBJ crtn.o 
