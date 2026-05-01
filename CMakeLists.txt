cmake_minimum_required(VERSION 3.14)
project(slcan-core)

option(BUILD_TESTING "Build tests" OFF)

add_library(slcan-core slcan.cpp)

target_include_directories(slcan-core PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

