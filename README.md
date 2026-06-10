# Multi-Sensor Data Logger over I2C/SPI

Multi-threaded C/C++ Linux data logger that reads temperature, pressure, and humidity sensors through /dev/i2c-* or /dev/spidev* and stores records in SQLite.

## Portfolio Purpose

This repository implements a host-testable C++17 model of a Linux multi-sensor data logger. It models I2C and SPI sensor descriptors, scripted `/dev/i2c-*` and `/dev/spidev*` reads, calibration, validation, SQLite-style transaction generation, and JSON telemetry for repeatable CI evidence.

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

Failure and warning scenarios:

```bash
./build/multi_sensor_i2c_spi_data_logger --bus-fault
./build/multi_sensor_i2c_spi_data_logger --out-of-range
./build/multi_sensor_i2c_spi_data_logger --sqlite-fail
```

## Implementation Slices

- C++17 data logger facade that captures one deterministic batch from I2C and SPI sensors.
- Sensor descriptors for a TMP102 temperature sensor, SHT31 humidity sensor, and BMP388 SPI pressure sensor.
- Calibration service that converts raw samples into engineering values.
- Strategy validation rules for bus health, scheduler guardrails, and value range warnings.
- SQLite statement store that emits a transaction suitable for later replacement with `sqlite3`.
- JSON telemetry encoder and text reporter for CI logs.
- CTest coverage for nominal capture, CRC/bus failure, warning-only range excursions, SQLite write failure, and report evidence.

## C++17 Design Patterns and SOLID

| Pattern | Implementation |
| --- | --- |
| Strategy | `IValidationRule` implementations validate bus health, schedule, and value ranges. |
| Composite | `CompositeValidator` runs the ordered validation chain. |
| Adapter | `ISensorBus`, `IDataStore`, and `IRecordEncoder` isolate hardware, persistence, and reporting. |
| Facade | `MultiSensorDataLogger::capture()` is the single batch-capture entry point. |
| Value Object | Sensor descriptors, raw samples, readings, issues, and log records are explicit structs. |

## Evidence Target

Linux file-based peripheral access, concurrent sensor reads, local persistence, and repeatable data-capture evidence.

## Remote

Intended public repository: https://github.com/rheslar1/multi-sensor-i2c-spi-data-logger

<!-- cpp17-solid-implementation:start -->
## C++17, Design Patterns, and SOLID Implementation

This repository includes a host-buildable C++17 implementation, not only documentation. The implementation applies:

- Strategy pattern for validation rules.
- Adapter interfaces for input samples and telemetry/reporting.
- Composite validation for combining safety and readiness checks.
- Facade orchestration through the project runtime class.
- SOLID boundaries between profile data, input acquisition, validation, telemetry encoding, and tests.
<!-- cpp17-solid-implementation:end -->

<!-- deep-architecture-links:start -->
## Deep Architecture and UML

- [Deep architecture](docs/deep-architecture.md)
- [Full UML Draw.io source](docs/diagrams/full-system-uml.drawio)
- [Full UML PNG export](docs/diagrams/full-system-uml.png)
<!-- deep-architecture-links:end -->
