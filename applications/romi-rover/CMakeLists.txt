cmake_minimum_required(VERSION 3.10)
project(romi-rover)

set(SOURCES main.cpp)

add_executable(romi-rover ${SOURCES})
target_link_libraries(romi-rover romi rover)
