SHELL = /bin/sh

.PHONY : all clean

CC := gcc

NAMES := cmemcpy cmemcpy2 cmemcpy3

BASE_FLAGS = -O3 -shared -fPIC -fomit-frame-pointer

# For a specific processor without avx support
ARCH_RAPTORLAKE = -march=raptorlake -mtune=raptorlake -mno-avx 

ARCH_SAPPHIRERAPIDS = -march=sapphirerapids -mtune=sapphirerapids

ARCH ?= $(ARCH_RAPTORLAKE)

A ?= R

ifeq ($(filter S s,$(A)),$(A))
	ARCH = $(ARCH_SAPPHIRERAPIDS)
endif

all : $(NAMES:%=%.so)

%.so : %.c
	$(CC) $< -o $@ $(BASE_FLAGS) $(ARCH)

clean :
	rm -f $(NAMES:%=%.so)
