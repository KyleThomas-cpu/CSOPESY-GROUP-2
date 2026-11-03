cmake_minimum_required(VERSION 3.20)
project(csopesy_mo1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(csopesy_mo1
        main.cpp
        config.cpp
        process.cpp
        report.cpp
        scheduler.cpp)

find_package(Threads REQUIRED)
target_link_libraries(csopesy_mo1 Threads::Threads)

configure_file(${CMAKE_SOURCE_DIR}/config.txt ${CMAKE_BINARY_DIR}/config.txt COPYONLY)