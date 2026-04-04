cmake_minimum_required(VERSION 3.14)

# set(CMAKE_CXX_STANDARD 17)
project(traffic_simulation)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")

find_package(OpenCV 4.1 REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIBRARY_DIRS})
add_definitions(${OpenCV_DEFINITIONS})

# Include src directory for headers
include_directories(src)

# Find all executables
file(GLOB project_SRCS src/*.cpp) #src/*.h

# Add project executable
add_executable(traffic_simulation ${project_SRCS})
target_link_libraries(traffic_simulation ${OpenCV_LIBRARIES})

# ==========================================
# Google Test Integration
# ==========================================
enable_testing()

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# ==========================================
# Test Executable
# ==========================================
# Source files for tests (exclude main simulation file)
set(test_SRCS
    tests/test_main.cpp
    tests/test_message_queue.cpp
    tests/test_traffic_light.cpp
    tests/test_intersection.cpp
    tests/test_vehicle.cpp
    tests/test_graphics.cpp
    tests/test_critical_fixes.cpp
    tests/test_high_priority_fixes.cpp
    tests/test_medium_priority_fixes.cpp
)

# Add test executable
add_executable(traffic_simulation_tests ${test_SRCS} src/TrafficLight.cpp src/TrafficObject.cpp src/Intersection.cpp src/Vehicle.cpp src/Street.cpp src/Graphics.cpp)

# Link test executable against gtest and OpenCV
target_link_libraries(traffic_simulation_tests gtest_main ${OpenCV_LIBRARIES})

# Include Google Test module for test discovery
include(GoogleTest)
# DISABLED: gtest_discover_tests causes issues with dynamic libraries on macOS
# Tests can be run manually with: ./traffic_simulation_tests
# gtest_discover_tests(traffic_simulation_tests)
