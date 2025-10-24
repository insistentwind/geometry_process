# Install script for directory: C:/D/qt_works/geometry_process/thirdparty/eigen-3.4.0/unsupported/Eigen

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/D/qt_works/geometry_process/out/install/x64-debug")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/eigen3/unsupported/Eigen" TYPE FILE FILES
    "C:/D/qt_works/geometry_process/thirdparty/eigen-3.4.0/unsupported/Eigen/AdolcForward"
    "C:/D/qt_works/geometry_process/thirdparty/eigen-3.4.0/unsupported/Eigen/AlignedVector3"
    "C:/D/qt_works/geometry_process/thirdparty/eigen-3.4.0/unsupported/Eigen/ArpackSupport"
    "C:/D/qt_works/geometry_process/thirdparty/eigen-3.4.0/unsupported/Eigen/AutoDiff"
    "C:/D/qt_works/geometry_process/thirdparty/eigen-3.4.0/unsupported/Eigen/BVH"
    "C:/D/qt_works/geometry_process/thirdparty/eigen-3.4.0/unsupported/Eigen/EulerAngles"
    "C:/D/qt_works/geometry_process/thirdparty/eigen-3.4.0/unsupported/Eigen/FFT"
    "C:/D/qt_works/geometry_process/thirdparty/eigen-3.4.0/unsupported/Eigen/IterativeSolvers"
    "C:/D/qt_works/geometry_process/thirdparty/eigen-3.4.0/unsupported/Eigen/KroneckerProduct"
    "C:/D/qt_works/geometry_process/thirdparty/eigen-3.4.0/unsupported/Eigen/LevenbergMarquardt"
    "C:/D/qt_works/geometry_process/thirdparty/eigen-3.4.0/unsupported/Eigen/MatrixFunctions"
    "C:/D/qt_works/geometry_process/thirdparty/eigen-3.4.0/unsupported/Eigen/MoreVectorization"
    "C:/D/qt_works/geometry_process/thirdparty/eigen-3.4.0/unsupported/Eigen/MPRealSupport"
    "C:/D/qt_works/geometry_process/thirdparty/eigen-3.4.0/unsupported/Eigen/NonLinearOptimization"
    "C:/D/qt_works/geometry_process/thirdparty/eigen-3.4.0/unsupported/Eigen/NumericalDiff"
    "C:/D/qt_works/geometry_process/thirdparty/eigen-3.4.0/unsupported/Eigen/OpenGLSupport"
    "C:/D/qt_works/geometry_process/thirdparty/eigen-3.4.0/unsupported/Eigen/Polynomials"
    "C:/D/qt_works/geometry_process/thirdparty/eigen-3.4.0/unsupported/Eigen/Skyline"
    "C:/D/qt_works/geometry_process/thirdparty/eigen-3.4.0/unsupported/Eigen/SparseExtra"
    "C:/D/qt_works/geometry_process/thirdparty/eigen-3.4.0/unsupported/Eigen/SpecialFunctions"
    "C:/D/qt_works/geometry_process/thirdparty/eigen-3.4.0/unsupported/Eigen/Splines"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/eigen3/unsupported/Eigen" TYPE DIRECTORY FILES "C:/D/qt_works/geometry_process/thirdparty/eigen-3.4.0/unsupported/Eigen/src" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/D/qt_works/geometry_process/out/build/x64-debug/thirdparty/eigen-3.4.0/unsupported/Eigen/CXX11/cmake_install.cmake")

endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/D/qt_works/geometry_process/out/build/x64-debug/thirdparty/eigen-3.4.0/unsupported/Eigen/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
