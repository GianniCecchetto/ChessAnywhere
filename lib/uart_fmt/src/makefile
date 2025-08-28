# ========================
# Makefile for cbproto lib
# ========================

CC      := gcc
CFLAGS  := -O2 -Wall -fPIC
LDFLAGS := -shared

SRC     := board_com.c
OBJ     := $(SRC:.c=.o)

# Nom du dossier de build
BUILDDIR := build
LIBNAME  := libcbproto.so
TARGET   := $(BUILDDIR)/$(LIBNAME)

.PHONY: all clean

all: $(TARGET)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(TARGET): $(OBJ) | $(BUILDDIR)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
