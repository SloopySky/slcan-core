add_library(slcan-core
    slcan.cpp
)

target_include_directories(slcan-core PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

