cmake_minimum_required(VERSION 3.16)
project(multi_sensor_i2c_spi_data_logger LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_library(data_logger_core
  src/DataLogger.cpp
)
target_include_directories(data_logger_core PUBLIC include)
target_compile_options(data_logger_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(${PROJECT_NAME}
  src/main.cpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE data_logger_core)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)

add_executable(data_logger_tests
  tests/DataLoggerTests.cpp
)
target_link_libraries(data_logger_tests PRIVATE data_logger_core)
target_compile_options(data_logger_tests PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME data_logger_tests COMMAND data_logger_tests)
