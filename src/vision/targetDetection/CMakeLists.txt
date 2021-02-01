set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)

# Find Packages
find_package(OpenCV REQUIRED)
find_package(wpilib REQUIRED)

# Executable
add_executable(targetDetection main.cpp)

# Linked Libraries
target_link_libraries(targetDetection ${OpenCV_LIBS} ntcore rambunctionVision)

target_include_directories(targetDetection PUBLIC ${PROJECT_SOURCE_DIR}/include)