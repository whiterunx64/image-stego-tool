.RECIPEPREFIX := >

CC      := gcc
CSTD    := -std=c11
WARN    := -Wall -Wextra
OPT     := -O2

PKGCFG  := $(shell pkg-config --exists libpng && echo yes)
ifeq ($(PKGCFG),yes)
    PNG_CFLAGS := $(shell pkg-config --cflags libpng)
    PNG_LIBS   := $(shell pkg-config --libs libpng)
else
    PNG_CFLAGS :=
    PNG_LIBS   := -lpng
endif

INCLUDES := -Iinclude
CFLAGS  := $(CSTD) $(WARN) $(OPT) $(INCLUDES) $(PNG_CFLAGS)
LDFLAGS :=
LIBS    := $(PNG_LIBS)

SRC_DIR := src

ENC_SRC := $(SRC_DIR)/lsb_encode.c
DEC_SRC := $(SRC_DIR)/lsb_decode.c

ENC_BIN := lsb-encode
DEC_BIN := lsb-decode

.PHONY: all
all: $(ENC_BIN) $(DEC_BIN)

$(ENC_BIN): $(ENC_SRC)
> $(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LIBS)

$(DEC_BIN): $(DEC_SRC)
> $(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LIBS)

.PHONY: clean
clean:
> $(RM) $(ENC_BIN) $(DEC_BIN)

.PHONY: debug
debug: CFLAGS := $(CSTD) $(WARN) -O0 -g $(INCLUDES) $(PNG_CFLAGS)
debug: clean all

.PHONY: release
release: CFLAGS := $(CSTD) $(WARN) -O3 $(INCLUDES) $(PNG_CFLAGS)
release: clean all

.PHONY: info
info:
> echo "Using pkg-config: $(PKGCFG)"
> echo "CFLAGS:  $(CFLAGS)"
> echo "LIBS:    $(LIBS)"
