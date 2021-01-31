set (CMAKE_CXX_STANDARD 17)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)

# Find Packages
find_package(OpenCV REQUIRED)
find_package(wpilib REQUIRED)

# Executable
add_executable(ballDetection main.cpp)

# Linked Libraries
target_link_libraries(ballDetection ${OpenCV_LIBS} ntcore rambunctionVision)

target_include_directories(ballDetection PUBLIC ${PROJECT_SOURCE_DIR}/include)