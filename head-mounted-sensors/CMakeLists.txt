cmake_minimum_required(VERSION 3.10.2)
project(sensors_transmitter)

add_executable(main src/main.cc src/serial.c)
target_compile_options(main PRIVATE -Wall -Wextra -pedantic)
