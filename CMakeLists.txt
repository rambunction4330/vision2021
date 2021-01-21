cmake_minimum_required(VERSION 3.0.0)
project(vision2021 VERSION 0.0.0)

# The project is set to build with C++17
set (CMAKE_CXX_STANDARD 17)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)

# Set Output Directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Sub directories
add_subdirectory(src/rambunctionVision)
add_subdirectory(src/vision/ballDetection)
add_subdirectory(src/vision/targetDetection)
add_subdirectory(src/tools/hsvTunning)
add_subdirectory(src/tools/cameraCalibration)