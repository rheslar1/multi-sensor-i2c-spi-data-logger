cmake_minimum_required(VERSION 3.16)
project(multi_sensor_i2c_spi_data_logger LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_executable(${PROJECT_NAME} src/main.cpp)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)

add_executable(profile_tests tests/ProfileTests.cpp)
target_compile_options(profile_tests PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME profile_tests COMMAND profile_tests)
