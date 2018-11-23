cmake_minimum_required(VERSION 3.10.2)
project(sensors_transmitter)

find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED)

add_executable(pimpolho src/9dof.cc src/main.cc src/serial.c)
target_compile_options(pimpolho PRIVATE -Wall -Wextra -pedantic)
target_link_libraries(pimpolho PRIVATE realsense RTIMULib Threads::Threads ${OpenCV_LIBS})
