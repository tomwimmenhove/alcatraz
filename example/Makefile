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
