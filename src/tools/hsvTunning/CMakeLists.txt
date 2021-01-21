set (CMAKE_CXX_STANDARD 17)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)

# Find Packages
find_package(OpenCV REQUIRED)

# Executable
add_executable(hsvTunning main.cpp)

# Linked Libraries
target_link_libraries(hsvTunning ${OpenCV_LIBS} rambunctionVision)

target_include_directories(hsvTunning PUBLIC ${PROJECT_SOURCE_DIR}/include)