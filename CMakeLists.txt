cmake_minimum_required(VERSION 3.14)
project(slcan-core)

option(BUILD_TESTING "Build tests" OFF)

add_library(slcan-core src/slcan.cpp)

target_include_directories(slcan-core PUBLIC include)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

