TARGET = udump.tos

CROSS = yes

ifeq ($(CROSS),yes)
  CROSSPREFIX=m68k-atari-mint-
else
  CROSSPREFIX=
endif

CC = $(CROSSPREFIX)gcc
LD = $(CROSSPREFIX)ld

CFLAGS = -m68000 -O2 -fomit-frame-pointer -Wall
LDFLAGS = -s

$(TARGET): udump.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f *~ *.o $(TARGET)
