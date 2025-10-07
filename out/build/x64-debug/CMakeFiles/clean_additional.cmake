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
  "viewer\\CMakeFiles\\mesh_viewer_autogen.dir\\AutogenUsed.txt"
  "viewer\\CMakeFiles\\mesh_viewer_autogen.dir\\ParseCache.txt"
  "viewer\\mesh_viewer_autogen"
  )
endif()
