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

OBJS = alcatraz.o
SRCS = alcatraz.cpp

all: libhost.a

%.o: %.cpp
	$(CPP) $(CPPFLAGS) -c $<
	$(POSTCOMPILE)

libhost.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

clean:
	rm -f *.o *.a

$(DEPDIR)/%.d: ;
	.PRECIOUS: $(DEPDIR)/%.d

include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS))))
