#!/bin/bash

mkdir build
cd build
cmake .. -DBUILD_TESTS=ON 
cmake --build .
# we want to be in the same directory as tictactoe.e
cd ..
./build/test/tictactoe
