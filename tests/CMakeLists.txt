# GoogleTest requires at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# googletest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(slcan-tests slmsg_test.cpp canmsg_test.cpp slcan_test.cpp msg_converter_test.cpp)
target_link_libraries(slcan-tests slcan-core GTest::gtest_main GTest::gmock)

include(GoogleTest)
gtest_discover_tests(slcan-tests)

