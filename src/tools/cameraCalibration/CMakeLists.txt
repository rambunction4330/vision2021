set (CMAKE_CXX_STANDARD 17)

# Find Packages
find_package(OpenCV REQUIRED)

# Executable
add_executable(calibrateCamera main.cpp)

# Linked Libraries
target_link_libraries(calibrateCamera ${OpenCV_LIBS} rambunctionVision)

target_include_directories(calibrateCamera PUBLIC ${PROJECT_SOURCE_DIR}/include)