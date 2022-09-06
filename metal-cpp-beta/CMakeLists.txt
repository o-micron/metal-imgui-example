cmake_minimum_required(VERSION 3.5)
project(MetalCpp VERSION 0.1.0 DESCRIPTION "MetalCpp" LANGUAGES C CXX OBJC OBJCXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(MetalCpp ${CMAKE_CURRENT_SOURCE_DIR}/lib.cpp)
target_include_directories(MetalCpp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(MetalCpp PRIVATE "-framework Foundation" "-framework QuartzCore" "-framework Metal")