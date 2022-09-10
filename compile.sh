#!/bin/bash

cmake --preset Darwin
cmake --build --preset Darwin --config Debug
cmake --build --preset Darwin --config Release