#!/bin/bash

git clone https://github.com/o-micron/imgui.git
git clone --depth 1 --single-branch -b release-2.0.22 https://github.com/libsdl-org/SDL.git

cd imgui
git checkout MetalCppAutoReleasePool
cd ../

cd metal-cpp-beta
mkdir build
cd build
mkdir Debug
mkdir Release
cd Debug
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_OSX_ARCHITECTURES=arm64 ../../
cmake --build . --config Debug
cd ..
cd Release
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64 ../../
cmake --build . --config Release
cd ../../../

cd SDL
mkdir build
cd build
mkdir Debug
mkdir Release
cd Debug
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=OFF -DCMAKE_OSX_ARCHITECTURES=arm64 ../../
cmake --build . --config Debug
cd ..
cd Release
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DCMAKE_OSX_ARCHITECTURES=arm64 ../../
cmake --build . --config Release
cd ../../../