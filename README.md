# metal-imgui-example
A metal-cpp playground project

[![CMake](https://github.com/o-micron/metal-imgui-example/actions/workflows/cmake.yml/badge.svg)](https://github.com/o-micron/metal-imgui-example/actions/workflows/cmake.yml)

# Instructions
```
# clone then build dependencies
./setup_dependencies.sh

# configure and build debug
cmake --preset DarwinDebug
cmake --build --preset DarwinDebug

# run and check for leaks
cd build/Debug
leaks --atExit -- ./metal-imgui-example
```
