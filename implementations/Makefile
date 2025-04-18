SHELL = /bin/sh

.PHONY : all clean s

CC := gcc

NAMES := cmemcpy cmemcpy2 cmemcpy3

# For a specific processor without avx support
FLAGS_RAPTORLAKE = -O3 -shared -fPIC -march=raptorlake -mtune=raptorlake -fomit-frame-pointer -mno-avx 

FLAGS_SAPPHIRERAPIDS = -O3 -shared -fPIC -march=sapphirerapids -mtune=sapphirerapids -fomit-frame-pointer

FLAGS ?= $(FLAGS_RAPTORLAKE)

all : $(NAMES:%=%.so)

%.so : %.c
	$(CC) $< -o $@ $(FLAGS) 

s: 
	make FLAGS="$(FLAGS_SAPPHIRERAPIDS)" all

clean :
	rm -f $(NAMES:%=%.so)
