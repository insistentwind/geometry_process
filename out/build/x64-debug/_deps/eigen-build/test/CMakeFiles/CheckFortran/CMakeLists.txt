cmake_minimum_required(VERSION 3.31.6-msvc6)
set(CMAKE_MODULE_PATH "E:/qt_work/myWork/out/build/x64-debug/_deps/eigen-src/cmake")
project(CheckFortran Fortran)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/result.cmake"
  "set(CMAKE_Fortran_COMPILER \"${CMAKE_Fortran_COMPILER}\")\n"
  
  )