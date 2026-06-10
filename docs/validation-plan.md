# Validation Plan

## Current Host Checks

- CMake configure completes.
- C++17 data logger core builds.
- Nominal executable captures three calibrated readings and emits SQLite transaction text.
- CTest validates nominal capture, CRC failure, range warning, SQLite write failure, and text/telemetry evidence.
- CLI scenarios cover bus fault, out-of-range data, and persistence failure.

## Host Commands

```bash
cmake -S . -B build
cmake --build build
./build/multi_sensor_i2c_spi_data_logger --nominal
./build/multi_sensor_i2c_spi_data_logger --bus-fault
./build/multi_sensor_i2c_spi_data_logger --out-of-range
./build/multi_sensor_i2c_spi_data_logger --sqlite-fail
ctest --test-dir build --output-on-failure
```

## Hardware Evidence To Add

- `i2cdetect` output for the TMP102 and SHT31 bus.
- `spidev` loopback or BMP388 read transcript.
- Timestamped SQLite database file and schema.
- Packet/log export showing one captured batch.
- Wiring photo showing I2C pull-ups and SPI chip select.

## Acceptance Criteria

- All configured sensors produce one raw sample per batch.
- CRC and bus-error flags are clean for accepted batches.
- Range warnings do not block persistence unless configured as critical later.
- SQLite transaction contains all accepted readings and commits atomically.

## Project-Specific Evidence Target

Linux file-based peripheral access, concurrent sensor reads, local persistence, and repeatable data-capture evidence.
