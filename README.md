# slcan-core

This is slcan-core component repository which implements [slcan protocol](https://www.canusb.com/files/canusb_manual.pdf)

## TODO

[ ] Refactor function names to **camelCase** for consistency
[ ] Refactor tests to use setup

## Consume with CMake

To cosume slcan-core component in a CMake project, it is recommended to use FetchContent.

```cmake
include(FetchContent)

FetchContent_Declare(slcan-core
  GIT_REPOSITORY https://github.com/SloopySky/slcan-core.git
  GIT_TAG        main #Note: Best practice to use specific git-hash or tagged version
)

FetchContent_MakeAvailable(slcan-core)
```

## Build and test

### Build slcan-core library

```
cmake -S . -B build
cmake --build build
```

### Build slcan-core library with tests

```
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
```

### Run tests

```
ctest --test-dir build/tests
```
