cmake_minimum_required(VERSION 2.6)
project(runTests)
 
# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

include_directories(include)
file(GLOB SOURCES "../src/tests/*.cpp")
add_executable(runTests ${SOURCES})

target_link_libraries(runTests ${GTEST_LIBRARIES} pthread)