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

To build **memcpy-bench** simply navigate first to respective subdirectories and use `make`

## Run

To run **memcpy-bench** go to `tests/` and use `make run`
