SHELL = /bin/sh

.PHONY : all clean

CC := gcc

SRC = $(wildcard *.c)
SO  = $(SRC:.c=.so)

BASE_FLAGS = -O3 -shared -fPIC -fomit-frame-pointer -ffreestanding -nostdlib 

# For a specific processor without avx support
ARCH_RAPTORLAKE = -march=raptorlake -mtune=raptorlake -mno-avx 

ARCH_SAPPHIRERAPIDS = -march=sapphirerapids -mtune=sapphirerapids

ARCH ?= $(ARCH_RAPTORLAKE)

A ?= R

ifeq ($(filter S s,$(A)),$(A))
	ARCH = $(ARCH_SAPPHIRERAPIDS)
endif

all : $(SO)

%.so : %.c
	$(CC) $< -o $@ $(BASE_FLAGS) $(ARCH)

clean :
	rm -f $(SO)
