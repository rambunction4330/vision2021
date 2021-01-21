set (CMAKE_CXX_STANDARD 17)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)

# Find Packages
find_package(OpenCV REQUIRED)

# Executable
add_executable(targetBuilder main.cpp)

# Linked Libraries
target_link_libraries(targetBuilder ${OpenCV_LIBS} rambunctionVision)

target_include_directories(targetBuilder PUBLIC ${PROJECT_SOURCE_DIR}/include)