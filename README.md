# Multi-Sensor Data Logger over I2C/SPI

Multi-threaded C/C++ Linux data logger that reads temperature, pressure, and humidity sensors through /dev/i2c-* or /dev/spidev* and stores records in SQLite.

## Portfolio Purpose

This repository is an Embedded Systems project scaffold for the Rheslar portfolio. It is designed to become a hardware-backed project with build output, validation logs, and reviewable implementation evidence.

All generated Embedded Systems repos are C++17-first and are framed around C++ design patterns and SOLID design principles.

## Stack

- C++17
- C++ Design Patterns
- SOLID
- I2C
- SPI
- POSIX threads
- SQLite
- C/C++
- Sensor logging

## Quick Start

```bash
cmake -S . -B build
cmake --build build
./build/multi_sensor_i2c_spi_data_logger
ctest --test-dir build --output-on-failure
```

## Implementation Slices

- C++17 starter executable that exposes the project identity, stack, and validation target.
- Small strategy-style readiness check that keeps the scaffold aligned with C++ design patterns.
- Architecture document with control boundaries, data flow, safety assumptions, and evidence plan.
- CTest smoke test that keeps source, docs, and CI files present as the repo grows.
- GitHub Actions workflow for configure, build, executable smoke run, and repository validation.

## Evidence Target

Linux file-based peripheral access, concurrent sensor reads, local persistence, and repeatable data-capture evidence.

## Remote

Intended public repository: https://github.com/rheslar1/multi-sensor-i2c-spi-data-logger
