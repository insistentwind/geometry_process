# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "src\\hw1\\CMakeFiles\\hw1_autogen.dir\\AutogenUsed.txt"
  "src\\hw1\\CMakeFiles\\hw1_autogen.dir\\ParseCache.txt"
  "src\\hw1\\hw1_autogen"
  "src\\hw2\\CMakeFiles\\hw2_autogen.dir\\AutogenUsed.txt"
  "src\\hw2\\CMakeFiles\\hw2_autogen.dir\\ParseCache.txt"
  "src\\hw2\\hw2_autogen"
  "src\\hw3\\CMakeFiles\\hw3_autogen.dir\\AutogenUsed.txt"
  "src\\hw3\\CMakeFiles\\hw3_autogen.dir\\ParseCache.txt"
  "src\\hw3\\hw3_autogen"
  "src\\hw4\\CMakeFiles\\hw4_autogen.dir\\AutogenUsed.txt"
  "src\\hw4\\CMakeFiles\\hw4_autogen.dir\\ParseCache.txt"
  "src\\hw4\\hw4_autogen"
  "src\\hw5\\CMakeFiles\\hw5_autogen.dir\\AutogenUsed.txt"
  "src\\hw5\\CMakeFiles\\hw5_autogen.dir\\ParseCache.txt"
  "src\\hw5\\hw5_autogen"
  "src\\hw6\\CMakeFiles\\hw6_autogen.dir\\AutogenUsed.txt"
  "src\\hw6\\CMakeFiles\\hw6_autogen.dir\\ParseCache.txt"
  "src\\hw6\\hw6_autogen"
  "src\\hw7\\CMakeFiles\\hw7_autogen.dir\\AutogenUsed.txt"
  "src\\hw7\\CMakeFiles\\hw7_autogen.dir\\ParseCache.txt"
  "src\\hw7\\hw7_autogen"
  "viewer\\CMakeFiles\\mesh_viewer_autogen.dir\\AutogenUsed.txt"
  "viewer\\CMakeFiles\\mesh_viewer_autogen.dir\\ParseCache.txt"
  "viewer\\mesh_viewer_autogen"
  )
endif()
