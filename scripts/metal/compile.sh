#!/bin/bash

mkdir $3

xcrun -sdk macosx metal -c $1 -o $3/$2.air
xcrun -sdk macosx metallib $3/$2.air -o $3/$2.metallib