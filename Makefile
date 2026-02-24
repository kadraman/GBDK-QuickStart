#
# GBC Template Makefile
# Builds a GameBoy Color ROM using GBDK-2020
#

# Clear environment variables that can interfere with the GBDK toolchain on Windows
INCLUDE :=
LIB :=
CPATH :=
export INCLUDE LIB CPATH

ifndef GBDK_HOME
    GBDK_HOME = $(HOME)/gbdk
endif

LCC         = $(GBDK_HOME)/bin/lcc
PNG2ASSET   = $(GBDK_HOME)/bin/png2asset

PROJECTNAME = GBCTemplate
SRCDIR      = src
OBJDIR      = obj
RESDIR      = res

# GBC mode flag + joined mode
LCCFLAGS    = -Wm-yc
LCCFLAGS   += -Wl-j

BINS        = $(OBJDIR)/$(PROJECTNAME).gbc

SRCSRC      = $(wildcard $(SRCDIR)/*.c)
RESSRC      = $(wildcard $(RESDIR)/*.c)
ALLSRC      = $(SRCSRC) $(RESSRC)
OBJS        = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCSRC)) \
              $(patsubst $(RESDIR)/%.c,$(OBJDIR)/%.o,$(RESSRC))

PNG_ASSETS  = $(RESDIR)/background.png $(RESDIR)/font.png $(RESDIR)/sprite.png

.PHONY: all convert clean

all: prepare $(BINS)

convert:
	$(PNG2ASSET) $(RESDIR)/background.png -c $(RESDIR)/background.c -map -bpp 2 -max_palettes 2
	$(PNG2ASSET) $(RESDIR)/font.png       -c $(RESDIR)/font.c       -map -bpp 2 -max_palettes 1
	$(PNG2ASSET) $(RESDIR)/sprite.png     -c $(RESDIR)/sprite.c          -bpp 2 -max_palettes 1 -spr8x16 -sw 8 -sh 16

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(LCC) $(LCCFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: $(RESDIR)/%.c
	$(LCC) $(LCCFLAGS) -c -o $@ $<

$(BINS): $(OBJS)
	$(LCC) $(LCCFLAGS) -o $@ $(OBJS)

prepare:
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR)
