# memcpy-bench

## Introduction

This project provides a benchmark suite for evaluating different memory copy strategies under varying conditions.

### Requirements

- **Linux OS**
- **Compiler**: GCC or Clang with C17 support (Raptorlake version tested and works on C99)
- **Build system**: `make`
- **Hardware**: It should run on most machines after adjusting `-march=raptorlake` and `-mtune=raptorlake` inside Makefiles. This benchmark was done for 2 platforms: raptor lake and sapphire rapids, as I wanted to test AVX512 and other extensions.
	You can use the [Intel Software Development Emulator], if you just want to see if they work.

### Project Structure

- `implementations/`: Contains different `memcpy` implementations
- `tests/`: Includes benchmarking tests for performance measurement and self-test

## Build

To build **memcpy-bench** use `make` in the main directory or do it separately in subdirectories.

## Run

To run **memcpy-bench** use `make run` in the main directory or use `make` or `make build` and then `make run` (main benchmark) or `make runt` (self_tests) `tests/` or navigate to subdirectories and do it separately.  
