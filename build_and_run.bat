mkdir build
cd build
cmake .. -G "MinGW Makefiles" 
cmake --build .
cd ..
.\build\src\tictactoe
