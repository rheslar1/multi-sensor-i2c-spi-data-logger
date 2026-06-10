#include "data_logger/DataLogger.hpp"

#include <algorithm>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace data_logger {
namespace {

std::string fixed2(const double value) {
  std::ostringstream stream;
  stream << std::fixed << std::setprecision(2) << value;
  return stream.str();
}

std::string sqlEscape(const std::string& value) {
  std::string escaped;
  for (const char item : value) {
    escaped += item;
    if (item == '\'') {
      escaped += '\'';
    }
  }
  return escaped;
}

std::string jsonEscape(const std::string& value) {
  std::ostringstream escaped;
  for (const char item : value) {
    switch (item) {
      case '\\':
        escaped << "\\\\";
        break;
      case '"':
        escaped << "\\\"";
        break;
      case '\n':
        escaped << "\\n";
        break;
      case '\r':
        escaped << "\\r";
        break;
      case '\t':
        escaped << "\\t";
        break;
      default:
        escaped << item;
        break;
    }
  }
  return escaped.str();
}

const SensorDescriptor* findSensor(const std::vector<SensorDescriptor>& sensors,
                                   const std::string& name) {
  const auto found = std::find_if(
      sensors.begin(),
      sensors.end(),
      [&name](const SensorDescriptor& sensor) {
        return sensor.name == name;
      });
  return found == sensors.end() ? nullptr : &(*found);
}

}  // namespace

std::string toString(const BusType bus) {
  return bus == BusType::I2c ? "i2c" : "spi";
}

std::string toString(const SensorKind kind) {
  switch (kind) {
    case SensorKind::Temperature:
      return "temperature";
    case SensorKind::Humidity:
      return "humidity";
    case SensorKind::Pressure:
      return "pressure";
  }
  return "unknown";
}

std::string toString(const Severity severity) {
  switch (severity) {
    case Severity::Info:
      return "info";
    case Severity::Warning:
      return "warning";
    case Severity::Critical:
      return "critical";
  }
  return "unknown";
}

ScriptedSensorBus::ScriptedSensorBus(std::vector<RawSample> samples)
    : samples_(std::move(samples)) {
  if (samples_.empty()) {
    throw std::invalid_argument("scripted sensor bus requires samples");
  }
}

std::vector<RawSample> ScriptedSensorBus::readBatch(const std::vector<SensorDescriptor>& sensors) {
  std::vector<RawSample> batch;
  for (const auto& sensor : sensors) {
    const auto found = std::find_if(
        samples_.begin(),
        samples_.end(),
        [&sensor](const RawSample& sample) {
          return sample.sensorName == sensor.name;
        });
    if (found != samples_.end()) {
      batch.push_back(*found);
    }
  }
  return batch;
}

SqliteStatementStore::SqliteStatementStore(const bool writable)
    : writable_(writable) {}

bool SqliteStatementStore::persist(const LogRecord& record, std::string& error) {
  if (!writable_) {
    error = "SQLite database is read-only";
    return false;
  }

  statements_.push_back("BEGIN IMMEDIATE TRANSACTION;");
  for (const auto& reading : record.readings) {
    std::ostringstream statement;
    statement << "INSERT INTO sensor_readings(batch_id, sensor, kind, bus, timestamp_ms, raw_value, engineering_value) VALUES("
              << record.batchId << ", '"
              << sqlEscape(reading.descriptor.name) << "', '"
              << toString(reading.descriptor.kind) << "', '"
              << toString(reading.descriptor.bus) << "', "
              << reading.raw.timestampMs << ", "
              << reading.raw.rawValue << ", "
              << fixed2(reading.engineeringValue) << ");";
    statements_.push_back(statement.str());
  }
  statements_.push_back("COMMIT;");
  error.clear();
  return true;
}

const std::vector<std::string>& SqliteStatementStore::statements() const {
  return statements_;
}

SensorReading CalibrationService::calibrate(
    const SensorDescriptor& descriptor,
    const RawSample& sample) const {
  return SensorReading{
      descriptor,
      sample,
      (static_cast<double>(sample.rawValue) * descriptor.scale) + descriptor.offset};
}

std::optional<LoggerIssue> BusHealthRule::evaluate(
    const std::vector<SensorDescriptor>& sensors,
    const std::vector<SensorReading>& readings) const {
  if (readings.size() != sensors.size()) {
    return LoggerIssue{Severity::Critical, "MISSING_SENSOR_SAMPLE", "not every configured sensor produced a sample"};
  }

  for (const auto& reading : readings) {
    if (reading.raw.busError) {
      return LoggerIssue{Severity::Critical, "BUS_ERROR", reading.descriptor.name + " reported a bus error"};
    }
    if (!reading.raw.crcOk) {
      return LoggerIssue{Severity::Critical, "CRC_ERROR", reading.descriptor.name + " failed sample CRC"};
    }
    if (reading.descriptor.devicePath.empty()) {
      return LoggerIssue{Severity::Critical, "DEVICE_PATH_MISSING", reading.descriptor.name + " has no Linux device path"};
    }
  }

  return std::nullopt;
}

std::string BusHealthRule::name() const {
  return "BusHealthRule";
}

std::optional<LoggerIssue> RangeRule::evaluate(
    const std::vector<SensorDescriptor>&,
    const std::vector<SensorReading>& readings) const {
  for (const auto& reading : readings) {
    if (reading.engineeringValue < reading.descriptor.minValue ||
        reading.engineeringValue > reading.descriptor.maxValue) {
      return LoggerIssue{
          Severity::Warning,
          "RANGE_WARNING",
          reading.descriptor.name + "=" + fixed2(reading.engineeringValue) +
              " outside [" + fixed2(reading.descriptor.minValue) + ", " +
              fixed2(reading.descriptor.maxValue) + "]"};
    }
  }
  return std::nullopt;
}

std::string RangeRule::name() const {
  return "RangeRule";
}

std::optional<LoggerIssue> ScheduleRule::evaluate(
    const std::vector<SensorDescriptor>& sensors,
    const std::vector<SensorReading>& readings) const {
  for (const auto& reading : readings) {
    const auto* sensor = findSensor(sensors, reading.raw.sensorName);
    if (sensor == nullptr) {
      return LoggerIssue{Severity::Critical, "UNKNOWN_SENSOR", reading.raw.sensorName + " is not configured"};
    }
    if (sensor->samplePeriodMs < 100U) {
      return LoggerIssue{Severity::Critical, "SAMPLE_PERIOD_TOO_FAST", sensor->name + " period is below scheduler guard"};
    }
  }
  return std::nullopt;
}

std::string ScheduleRule::name() const {
  return "ScheduleRule";
}

void CompositeValidator::add(std::unique_ptr<IValidationRule> rule) {
  if (!rule) {
    throw std::invalid_argument("validation rule cannot be null");
  }
  rules_.push_back(std::move(rule));
}

std::vector<LoggerIssue> CompositeValidator::evaluate(
    const std::vector<SensorDescriptor>& sensors,
    const std::vector<SensorReading>& readings) const {
  std::vector<LoggerIssue> issues;
  for (const auto& rule : rules_) {
    if (const auto issue = rule->evaluate(sensors, readings)) {
      issues.push_back(*issue);
    }
  }
  return issues;
}

std::vector<std::string> CompositeValidator::trace() const {
  std::vector<std::string> names;
  for (const auto& rule : rules_) {
    names.push_back(rule->name());
  }
  return names;
}

std::string JsonRecordEncoder::encode(const LogRecord& record) const {
  std::ostringstream payload;
  payload << "{\"batchId\":" << record.batchId
          << ",\"accepted\":" << (record.accepted ? "true" : "false")
          << ",\"issueCount\":" << record.issues.size()
          << ",\"readings\":[";
  for (std::size_t index = 0; index < record.readings.size(); ++index) {
    const auto& reading = record.readings[index];
    if (index > 0U) {
      payload << ',';
    }
    payload << "{\"sensor\":\"" << jsonEscape(reading.descriptor.name)
            << "\",\"kind\":\"" << toString(reading.descriptor.kind)
            << "\",\"bus\":\"" << toString(reading.descriptor.bus)
            << "\",\"value\":" << fixed2(reading.engineeringValue)
            << ",\"timestampMs\":" << reading.raw.timestampMs << '}';
  }
  payload << "]}";
  return payload.str();
}

MultiSensorDataLogger::MultiSensorDataLogger(
    std::vector<SensorDescriptor> sensors,
    ISensorBus& bus,
    IDataStore& store,
    CompositeValidator validator,
    CalibrationService calibration,
    const IRecordEncoder& encoder)
    : sensors_(std::move(sensors)),
      bus_(bus),
      store_(store),
      validator_(std::move(validator)),
      calibration_(calibration),
      encoder_(encoder) {}

LogRecord MultiSensorDataLogger::capture(const std::uint32_t batchId) {
  LogRecord record;
  record.batchId = batchId;
  record.trace = validator_.trace();

  const auto rawSamples = bus_.readBatch(sensors_);
  for (const auto& sample : rawSamples) {
    const auto* descriptor = findSensor(sensors_, sample.sensorName);
    if (descriptor != nullptr) {
      record.readings.push_back(calibration_.calibrate(*descriptor, sample));
    }
  }

  record.issues = validator_.evaluate(sensors_, record.readings);
  record.accepted = std::none_of(
      record.issues.begin(),
      record.issues.end(),
      [](const LoggerIssue& issue) {
        return issue.severity == Severity::Critical;
      });

  std::string storeError;
  if (record.accepted && !store_.persist(record, storeError)) {
    record.accepted = false;
    record.issues.push_back(LoggerIssue{Severity::Critical, "SQLITE_WRITE_FAILED", storeError});
  }

  const auto& statements = store_.statements();
  std::ostringstream transaction;
  for (const auto& statement : statements) {
    transaction << statement << '\n';
  }
  record.sqliteTransaction = transaction.str();
  record.telemetryPayload = encoder_.encode(record);
  return record;
}

TextLogReporter::TextLogReporter(std::ostream& stream) : stream_(stream) {}

void TextLogReporter::publish(const LogRecord& record) const {
  stream_ << "batch=" << record.batchId
          << " status=" << (record.accepted ? "PASS" : "FAIL")
          << " readings=" << record.readings.size() << '\n';
  for (const auto& rule : record.trace) {
    stream_ << "rule=" << rule << '\n';
  }
  for (const auto& reading : record.readings) {
    stream_ << "reading=" << reading.descriptor.name
            << " bus=" << toString(reading.descriptor.bus)
            << " value=" << fixed2(reading.engineeringValue)
            << '\n';
  }
  for (const auto& issue : record.issues) {
    stream_ << "issue=" << toString(issue.severity) << ':'
            << issue.code << ':' << issue.detail << '\n';
  }
  stream_ << "sqlite_begin\n" << record.sqliteTransaction
          << "sqlite_end\n"
          << "telemetry=" << record.telemetryPayload << '\n';
}

std::vector<SensorDescriptor> demoSensors() {
  return {
      {"tmp102-zone", SensorKind::Temperature, BusType::I2c, "/dev/i2c-1", 0x48U, 1000U, 0.0625, 0.0, -20.0, 85.0},
      {"sht31-room", SensorKind::Humidity, BusType::I2c, "/dev/i2c-1", 0x44U, 1000U, 0.01, 0.0, 0.0, 100.0},
      {"bmp388-barometer", SensorKind::Pressure, BusType::Spi, "/dev/spidev0.0", 0U, 1000U, 0.001, 0.0, 80.0, 120.0}};
}

std::vector<RawSample> nominalSamples() {
  return {
      {"tmp102-zone", 1000U, 384, true, false},
      {"sht31-room", 1000U, 4550, true, false},
      {"bmp388-barometer", 1000U, 101325, true, false}};
}

std::vector<RawSample> busFaultSamples() {
  auto samples = nominalSamples();
  samples[1].crcOk = false;
  return samples;
}

std::vector<RawSample> outOfRangeSamples() {
  auto samples = nominalSamples();
  samples[0].rawValue = 2400;
  return samples;
}

CompositeValidator defaultValidator() {
  CompositeValidator validator;
  validator.add(std::make_unique<BusHealthRule>());
  validator.add(std::make_unique<ScheduleRule>());
  validator.add(std::make_unique<RangeRule>());
  return validator;
}

LogRecord runScenario(const std::string& scenario) {
  std::vector<RawSample> samples;
  bool writable = true;
  if (scenario == "nominal") {
    samples = nominalSamples();
  } else if (scenario == "bus-fault") {
    samples = busFaultSamples();
  } else if (scenario == "out-of-range") {
    samples = outOfRangeSamples();
  } else if (scenario == "sqlite-fail") {
    samples = nominalSamples();
    writable = false;
  } else {
    throw std::invalid_argument("unknown scenario: " + scenario);
  }

  ScriptedSensorBus bus(samples);
  SqliteStatementStore store(writable);
  JsonRecordEncoder encoder;
  MultiSensorDataLogger logger(
      demoSensors(),
      bus,
      store,
      defaultValidator(),
      CalibrationService{},
      encoder);
  return logger.capture(42U);
}

}  // namespace data_logger
