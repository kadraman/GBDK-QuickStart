#
# GBDK QuickStart Template
# Builds a GameBoy Color ROM using GBDK-2020
#

# Clear environment variables that can interfere with the GBDK toolchain on Windows
INCLUDE :=
LIB :=
CPATH :=
export INCLUDE LIB CPATH

# Emulicious executable (can be overridden via env or CLI)
# On Windows prefer the bundled `Emulicious.exe`.
# On Unix-like systems prefer a system `Emulicious` binary if available,
# otherwise fall back to `java -jar Emulicious.jar` if that file exists.
ifeq ($(OS),Windows_NT)
EMULICIOUS ?= Emulicious.exe
else
EMULICIOUS ?= $(shell (command -v Emulicious >/dev/null 2>&1 && echo Emulicious) || \
				(command -v emulicious >/dev/null 2>&1 && echo emulicious) || \
				(test -f Emulicious.jar && echo "java -jar Emulicious.jar") || \
				echo Emulicious)
endif

# Detect python command: prefer `python3`, fall back to `python`.
# Allow users to override by setting `PYTHON` in the environment or CLI: `make PYTHON=python3`.
PYTHON ?= $(shell (command -v python3 >/dev/null 2>&1 && echo python3) || \
			(command -v python >/dev/null 2>&1 && echo python) || \
			echo python)

ifndef GBDK_HOME
    GBDK_HOME = $(HOME)/gbdk
endif

LCC         = $(GBDK_HOME)/bin/lcc
PNG2ASSET   = $(GBDK_HOME)/bin/png2asset

PROJECTNAME = quickstart
SRCDIR      = src
OBJDIR      = obj
RESDIR      = res

# GBC mode flag + joined mode
LCCFLAGS    = -Wm-yc
LCCFLAGS   += -Wl-j
# MBC5 cartridge type (supports up to 512 ROM banks, recommended for GBC)
LCCFLAGS   += -Wm-yt25
# Auto banking: when a bank is full the linker automatically overflows
# data/code into the next bank.  Code that accesses banked data must call
# SWITCH_ROM(BANK(symbol)) / SWITCH_ROM(1) around each access.
LCCFLAGS   += -Wm-ybo

BINS        = $(OBJDIR)/$(PROJECTNAME).gbc

# Source directories (split library vs game-specific folders)
SRCDIRS     = $(SRCDIR)/lib/src $(SRCDIR)/game $(SRCDIR)/game/states $(SRCDIR)/game/sprites
SRCSRC      = $(foreach d,$(SRCDIRS),$(wildcard $(d)/*.c))
RESSRC      = $(wildcard $(RESDIR)/*.c)
ALLSRC      = $(SRCSRC) $(RESSRC)
OBJS        = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCSRC)) \
			  $(patsubst $(RESDIR)/%.c,$(OBJDIR)/%.o,$(RESSRC))

# Include search paths for lcc
INCLUDES    = -I$(SRCDIR)/lib/include -I$(SRCDIR)/game -I$(SRCDIR)/game/states -I$(SRCDIR)/game/sprites -I$(RESDIR)

PNG_ASSETS  = $(RESDIR)/background.png $(RESDIR)/font.png $(RESDIR)/player.png $(RESDIR)/enemy.png \
              $(RESDIR)/bg_title.png $(RESDIR)/bg_gameover.png $(RESDIR)/bg_win.png

.PHONY: all generate convert clean clean-generated run

all: prepare $(BINS)

# Generate PNG assets + C/H source files using Python scripts (no GBDK needed).
# Requires: pip install pillow
generate:
	$(PYTHON) tools/generate_assets.py

# Convert PNG assets to GBDK-compatible C source files using png2asset.
convert:
	$(PNG2ASSET) $(RESDIR)/background.png  -c $(RESDIR)/background.c  -map -bpp 2 -max_palettes 2
	$(PNG2ASSET) $(RESDIR)/bg_title.png    -c $(RESDIR)/bg_title.c    -map -bpp 2 -max_palettes 2
	$(PNG2ASSET) $(RESDIR)/bg_gameover.png -c $(RESDIR)/bg_gameover.c -map -bpp 2 -max_palettes 2
	$(PNG2ASSET) $(RESDIR)/bg_win.png      -c $(RESDIR)/bg_win.c      -map -bpp 2 -max_palettes 2
	$(PNG2ASSET) $(RESDIR)/font.png        -c $(RESDIR)/font.c        -map -bpp 2 -max_palettes 1
	$(PNG2ASSET) $(RESDIR)/player.png      -c $(RESDIR)/player.c           -bpp 2 -max_palettes 1 -spr8x16 -sw 8 -sh 16
	$(PNG2ASSET) $(RESDIR)/enemy.png       -c $(RESDIR)/enemy.c            -bpp 2 -max_palettes 1 -spr8x8  -sw 8 -sh 8

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(LCC) $(LCCFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJDIR)/%.o: $(RESDIR)/%.c | $(OBJDIR)
	$(LCC) $(LCCFLAGS) -c -o $@ $<

$(BINS): $(OBJS)
	$(LCC) $(LCCFLAGS) -o $@ $(OBJS)

prepare:
	mkdir -p $(OBJDIR)

run: $(BINS)
ifeq ($(OS),Windows_NT)
	# Use PowerShell Start-Process to reliably launch GUI apps from Make
	powershell -NoProfile -Command Start-Process -FilePath "$(EMULICIOUS)" -ArgumentList "$(BINS)"
else
	$(EMULICIOUS) "$(BINS)" &
endif

clean:
	rm -rf $(OBJDIR)

# Remove only build artifacts; use `make clean-generated` to remove generated
# asset sources in `res/` (background, font, sprite).
clean-generated:
	# Remove generated asset sources in res/ (backgrounds, fonts, sprites)
	rm -f $(RESDIR)/background.* $(RESDIR)/bg_title.* $(RESDIR)/bg_gameover.* $(RESDIR)/bg_win.*
	rm -f $(RESDIR)/font.* $(RESDIR)/player.* $(RESDIR)/enemy.*
