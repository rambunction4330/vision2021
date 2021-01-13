set (CMAKE_CXX_STANDARD 17)

# Find Packages
find_package(OpenCV REQUIRED)

# Executable
add_executable(hsvTunning main.cpp)

# Linked Libraries
target_link_libraries(hsvTunning ${OpenCV_LIBS})