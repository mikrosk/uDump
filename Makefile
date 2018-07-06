TARGET = udump.tos

CROSS = yes
LIBCMINI = ../libcmini.git

ifeq ($(CROSS),yes)
  CROSSPREFIX=m68k-atari-mint-
else
  CROSSPREFIX=
endif

CC = $(CROSSPREFIX)gcc
LD = $(CROSSPREFIX)ld

CFLAGS = -m68000 -O2 -fomit-frame-pointer -Wall -I$(LIBCMINI)/include -nostdlib
LDFLAGS = -s -L$(LIBCMINI) -lcmini -nostdlib -lgcc

$(TARGET): udump.c
	$(CC) $(CFLAGS) -o $@ $(LIBCMINI)/startup.o  $< $(LDFLAGS)

clean:
	rm -f *~ *.o $(TARGET)
