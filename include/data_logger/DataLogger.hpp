#ifndef DATA_LOGGER_DATA_LOGGER_HPP_
#define DATA_LOGGER_DATA_LOGGER_HPP_

#include <cstdint>
#include <iosfwd>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace data_logger {

enum class BusType {
  I2c,
  Spi
};

enum class SensorKind {
  Temperature,
  Humidity,
  Pressure
};

enum class Severity {
  Info,
  Warning,
  Critical
};

std::string toString(BusType bus);
std::string toString(SensorKind kind);
std::string toString(Severity severity);

struct SensorDescriptor {
  std::string name;
  SensorKind kind{SensorKind::Temperature};
  BusType bus{BusType::I2c};
  std::string devicePath;
  std::uint16_t addressOrChipSelect{};
  std::uint32_t samplePeriodMs{1000};
  double scale{1.0};
  double offset{};
  double minValue{};
  double maxValue{};
};

struct RawSample {
  std::string sensorName;
  std::uint64_t timestampMs{};
  std::int32_t rawValue{};
  bool crcOk{true};
  bool busError{};
};

struct SensorReading {
  SensorDescriptor descriptor;
  RawSample raw;
  double engineeringValue{};
};

struct LoggerIssue {
  Severity severity{Severity::Critical};
  std::string code;
  std::string detail;
};

struct LogRecord {
  std::uint32_t batchId{};
  bool accepted{};
  std::vector<SensorReading> readings;
  std::vector<LoggerIssue> issues;
  std::vector<std::string> trace;
  std::string sqliteTransaction;
  std::string telemetryPayload;
};

class ISensorBus {
 public:
  virtual ~ISensorBus() = default;
  virtual std::vector<RawSample> readBatch(const std::vector<SensorDescriptor>& sensors) = 0;
};

class IDataStore {
 public:
  virtual ~IDataStore() = default;
  virtual bool persist(const LogRecord& record, std::string& error) = 0;
  virtual const std::vector<std::string>& statements() const = 0;
};

class IRecordEncoder {
 public:
  virtual ~IRecordEncoder() = default;
  virtual std::string encode(const LogRecord& record) const = 0;
};

class IValidationRule {
 public:
  virtual ~IValidationRule() = default;
  virtual std::optional<LoggerIssue> evaluate(
      const std::vector<SensorDescriptor>& sensors,
      const std::vector<SensorReading>& readings) const = 0;
  virtual std::string name() const = 0;
};

class ScriptedSensorBus final : public ISensorBus {
 public:
  explicit ScriptedSensorBus(std::vector<RawSample> samples);
  std::vector<RawSample> readBatch(const std::vector<SensorDescriptor>& sensors) override;

 private:
  std::vector<RawSample> samples_;
};

class SqliteStatementStore final : public IDataStore {
 public:
  explicit SqliteStatementStore(bool writable = true);

  bool persist(const LogRecord& record, std::string& error) override;
  const std::vector<std::string>& statements() const override;

 private:
  bool writable_{};
  std::vector<std::string> statements_;
};

class CalibrationService final {
 public:
  SensorReading calibrate(const SensorDescriptor& descriptor, const RawSample& sample) const;
};

class BusHealthRule final : public IValidationRule {
 public:
  std::optional<LoggerIssue> evaluate(
      const std::vector<SensorDescriptor>& sensors,
      const std::vector<SensorReading>& readings) const override;
  std::string name() const override;
};

class RangeRule final : public IValidationRule {
 public:
  std::optional<LoggerIssue> evaluate(
      const std::vector<SensorDescriptor>& sensors,
      const std::vector<SensorReading>& readings) const override;
  std::string name() const override;
};

class ScheduleRule final : public IValidationRule {
 public:
  std::optional<LoggerIssue> evaluate(
      const std::vector<SensorDescriptor>& sensors,
      const std::vector<SensorReading>& readings) const override;
  std::string name() const override;
};

class CompositeValidator final {
 public:
  void add(std::unique_ptr<IValidationRule> rule);
  std::vector<LoggerIssue> evaluate(
      const std::vector<SensorDescriptor>& sensors,
      const std::vector<SensorReading>& readings) const;
  std::vector<std::string> trace() const;

 private:
  std::vector<std::unique_ptr<IValidationRule>> rules_;
};

class JsonRecordEncoder final : public IRecordEncoder {
 public:
  std::string encode(const LogRecord& record) const override;
};

class MultiSensorDataLogger final {
 public:
  MultiSensorDataLogger(std::vector<SensorDescriptor> sensors,
                        ISensorBus& bus,
                        IDataStore& store,
                        CompositeValidator validator,
                        CalibrationService calibration,
                        const IRecordEncoder& encoder);

  LogRecord capture(std::uint32_t batchId);

 private:
  std::vector<SensorDescriptor> sensors_;
  ISensorBus& bus_;
  IDataStore& store_;
  CompositeValidator validator_;
  CalibrationService calibration_;
  const IRecordEncoder& encoder_;
};

class TextLogReporter final {
 public:
  explicit TextLogReporter(std::ostream& stream);
  void publish(const LogRecord& record) const;

 private:
  std::ostream& stream_;
};

std::vector<SensorDescriptor> demoSensors();
std::vector<RawSample> nominalSamples();
std::vector<RawSample> busFaultSamples();
std::vector<RawSample> outOfRangeSamples();
CompositeValidator defaultValidator();
LogRecord runScenario(const std::string& scenario);

}  // namespace data_logger

#endif  // DATA_LOGGER_DATA_LOGGER_HPP_
