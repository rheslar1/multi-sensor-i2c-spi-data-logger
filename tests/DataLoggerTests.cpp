#include "data_logger/DataLogger.hpp"

#include <cassert>
#include <sstream>
#include <string>

namespace {

bool contains(const std::string& value, const std::string& needle) {
  return value.find(needle) != std::string::npos;
}

bool hasIssue(const data_logger::LogRecord& record, const std::string& code) {
  for (const auto& issue : record.issues) {
    if (issue.code == code) {
      return true;
    }
  }
  return false;
}

void nominalCapturePersistsThreeReadings() {
  const auto record = data_logger::runScenario("nominal");

  assert(record.accepted);
  assert(record.readings.size() == 3U);
  assert(record.issues.empty());
  assert(contains(record.sqliteTransaction, "BEGIN IMMEDIATE TRANSACTION"));
  assert(contains(record.sqliteTransaction, "tmp102-zone"));
  assert(contains(record.telemetryPayload, "\"accepted\":true"));
}

void busFaultRejectsBatch() {
  const auto record = data_logger::runScenario("bus-fault");

  assert(!record.accepted);
  assert(hasIssue(record, "CRC_ERROR"));
  assert(record.sqliteTransaction.empty());
}

void outOfRangeWarningStillPersists() {
  const auto record = data_logger::runScenario("out-of-range");

  assert(record.accepted);
  assert(hasIssue(record, "RANGE_WARNING"));
  assert(contains(record.sqliteTransaction, "COMMIT"));
}

void sqliteFailureRejectsBatch() {
  const auto record = data_logger::runScenario("sqlite-fail");

  assert(!record.accepted);
  assert(hasIssue(record, "SQLITE_WRITE_FAILED"));
}

void textReportIncludesRulesAndTelemetry() {
  const auto record = data_logger::runScenario("nominal");
  std::ostringstream output;
  data_logger::TextLogReporter reporter(output);

  reporter.publish(record);

  assert(contains(output.str(), "rule=BusHealthRule"));
  assert(contains(output.str(), "reading=bmp388-barometer"));
  assert(contains(output.str(), "telemetry="));
}

}  // namespace

int main() {
  nominalCapturePersistsThreeReadings();
  busFaultRejectsBatch();
  outOfRangeWarningStillPersists();
  sqliteFailureRejectsBatch();
  textReportIncludesRulesAndTelemetry();
  return 0;
}
