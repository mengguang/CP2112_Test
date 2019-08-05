cmake_minimum_required(VERSION 3.14)
project(CP2112_Test C)

set(CMAKE_C_STANDARD 99)
link_directories(.)
add_executable(CP2112_Test main.c SLABCP2112.h Types.h silabs_sal.h silabs_defs.h)
target_link_libraries(CP2112_Test SLABHIDDevice SLABHIDtoSMBus)