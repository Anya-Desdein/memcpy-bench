# memcpy-bench

## Introduction

This project provides a benchmark suite for evaluating different memory copy strategies under varying conditions.

### Requirements

- **Linux OS**
- **Compiler**: GCC or Clang with C23 support
- **Build system**: `make`
- **Hardware**: Intel CPU with AVX, AVX2, AVX-512 support
	Alternatively, use the [Intel Software Development Emulator]

### Project Structure

- `implementations/`: Contains different `memcpy` implementations
- `tests/`: Includes benchmarking tests for performance measurement and self-test

## Build

To build **memcpy-bench** simply navigate first to respective subdirectories and use `make`

## Run

To run **memcpy-bench** go to `tests/` and use `make run`
