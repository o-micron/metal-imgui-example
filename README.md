# metal-imgui-example

# Build
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
