mkdir build
cd build
cmake .. -DBUILD_TESTS=ON -G "MinGW Makefiles" 
cmake --build .
REM we want to be in the same directory as tictactoe.e
cd ..
.\build\test\tictactoe
