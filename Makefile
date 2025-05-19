TARGET = udump

CROSS := yes

ifeq ($(CROSS),yes)
  CROSSPREFIX=m68k-atari-mint-
else
  CROSSPREFIX=
endif

CC := $(CROSSPREFIX)gcc
LIBCMINI_ROOT := $(shell $(CC) -print-sysroot)/opt/libcmini

CFLAGS  := -m68000 -O2 -fomit-frame-pointer -Wall -I$(LIBCMINI_ROOT)/include
LDFLAGS	:= -s -nostdlib $(LIBCMINI_ROOT)/lib/crt0.o $(LDFLAGS) -L$(LIBCMINI_ROOT)/lib
LDLIBS  := -lgcc -lcmini -lgcc

$(TARGET).tos: $(TARGET)
	cp -a $< $@

$(TARGET): udump.o

clean:
	rm -f *~ *.o $(TARGET) $(TARGET).tos
