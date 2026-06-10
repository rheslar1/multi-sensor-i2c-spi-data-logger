cmake_minimum_required(VERSION 3.16)
project(multi_sensor_i2c_spi_data_logger C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} src/main.c)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
