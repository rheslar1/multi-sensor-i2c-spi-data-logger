#include "data_logger/DataLogger.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void usage(const char* programName) {
  std::cout << "Usage: " << programName << " [--nominal|--bus-fault|--out-of-range|--sqlite-fail|--help]\n";
}

std::string scenarioFromOption(const std::string& option) {
  if (option == "--nominal") {
    return "nominal";
  }
  if (option == "--bus-fault") {
    return "bus-fault";
  }
  if (option == "--out-of-range") {
    return "out-of-range";
  }
  if (option == "--sqlite-fail") {
    return "sqlite-fail";
  }
  return {};
}

}  // namespace

int main(int argc, char** argv) {
  const std::string option = argc > 1 ? argv[1] : "--nominal";
  if (option == "--help") {
    usage(argv[0]);
    return 0;
  }

  const std::string scenario = scenarioFromOption(option);
  if (scenario.empty()) {
    usage(argv[0]);
    return 1;
  }

  try {
    const auto record = data_logger::runScenario(scenario);
    data_logger::TextLogReporter reporter(std::cout);
    reporter.publish(record);
    return record.accepted ? 0 : 2;
  } catch (const std::exception& exception) {
    std::cerr << "data logger error: " << exception.what() << '\n';
    return 1;
  }
}
