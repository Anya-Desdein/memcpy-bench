SHELL = /bin/sh

.PHONY: mem build run runt link clean cleant

MEMD = "./implementations"
RUND = "./tests"

all: build link 

mem: 
	$(MAKE) -C $(MEMD)

build:
	$(MAKE) -C $(MEMD)
	$(MAKE) -C $(RUND)

run: build link 
	$(MAKE) -C $(RUND) run

runt: build link
	$(MAKE) -C $(RUND) runt

link: mem
	$(MAKE) -C $(RUND) link

clean:
	$(MAKE) -C $(RUND) clean

cleant: 
	$(MAKE) -C $(RUND) cleant
