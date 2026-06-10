# Multi-Sensor Data Logger over I2C/SPI Architecture

## Goal

Linux file-based peripheral access, concurrent sensor reads, local persistence, and repeatable data-capture evidence.

## Runtime Shape

1. Configure sensor descriptors for I2C and SPI devices.
2. Read a batch through an `ISensorBus` adapter.
3. Convert raw values through `CalibrationService`.
4. Run validation strategies for bus health, scheduler constraints, and engineering range.
5. Persist accepted batches through `IDataStore`.
6. Emit SQLite-style transaction text, telemetry JSON, and human-readable reports.

## C++17 Design Shape

- `MultiSensorDataLogger` is the facade for batch capture.
- `ISensorBus`, `IDataStore`, and `IRecordEncoder` are adapter interfaces.
- `BusHealthRule`, `ScheduleRule`, and `RangeRule` are strategy objects.
- `CompositeValidator` combines independent validation rules.
- `SensorDescriptor`, `RawSample`, `SensorReading`, and `LogRecord` are value objects.

## SOLID Notes

- Single Responsibility: bus reads, calibration, validation, storage, encoding, and reporting are separate.
- Open/Closed: new sensor checks can be added as validation rules.
- Liskov Substitution: hardware bus readers can replace `ScriptedSensorBus`.
- Interface Segregation: bus, storage, encoding, and validation contracts are narrow.
- Dependency Inversion: the logger depends on interfaces and injected collaborators.

## Host-to-Hardware Mapping

| Host Model | Linux Target Mapping |
| --- | --- |
| `SensorDescriptor.devicePath` | `/dev/i2c-1`, `/dev/spidev0.0` |
| `ScriptedSensorBus` | future `ioctl(I2C_RDWR)` and `SPI_IOC_MESSAGE` adapter |
| `SqliteStatementStore` | future `sqlite3_prepare_v2` / transaction writer |
| `JsonRecordEncoder` | MQTT/HTTP/local log payload |
| CTest scenarios | repeatable CI gate before hardware is attached |

## Validation Plan

- Build the host model with CMake.
- Run nominal capture and verify three readings persist.
- Run bus fault, out-of-range, and SQLite failure scenarios.
- Add real sensor hardware logs once target wiring is available.

<!-- deep-architecture-links:start -->
## Deep Architecture and UML

- [Deep architecture](docs/deep-architecture.md)
- [Full UML Draw.io source](docs/diagrams/full-system-uml.drawio)
- [Full UML PNG export](docs/diagrams/full-system-uml.png)
<!-- deep-architecture-links:end -->
