set (CMAKE_CXX_STANDARD 17)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)

# Find Packages
find_package(OpenCV REQUIRED)

# Executable
add_library(rambunctionVision imageProcessing.cpp contourprocessing.cpp drawing.cpp)

# Linked Libraries
target_link_libraries(rambunctionVision ${OpenCV_LIBS})

# Directories to include
target_include_directories(rambunctionVision PUBLIC ${PROJECT_SOURCE_DIR}/include)
