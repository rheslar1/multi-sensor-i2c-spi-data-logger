# Multi-Sensor Data Logger over I2C/SPI Design Package

## Purpose

Multi-threaded C/C++ Linux data logger that reads temperature, pressure, and humidity sensors through /dev/i2c-* or /dev/spidev* and stores records in SQLite.

This package defines the project as an implementation-ready embedded system. It covers system architecture, requirements, interface boundaries, runtime design, validation evidence, and phased delivery.

## Project Profile

| Field | Value |
| --- | --- |
| Repository | `rheslar1/multi-sensor-i2c-spi-data-logger` |
| Primary stack | C++17, C++ Design Patterns, SOLID, I2C, SPI, POSIX threads, SQLite, C/C++, Sensor logging |
| Review proof point | Linux file-based peripheral access, concurrent sensor reads, local persistence, and repeatable data-capture evidence. |

## Artifacts

- [System Design](system-design.md)
- [Requirements](requirements.md)
- [Interface Control](interface-control.md)
- [Runtime Design](runtime-design.md)
- [Validation Plan](validation-plan.md)
- [Implementation Roadmap](implementation-roadmap.md)
- [Draw.io UML](diagrams/system-design.drawio)
- [PNG UML](diagrams/system-design.png)
