cmake_minimum_required(VERSION 3.12)
project(DFMPatternCapture)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

option(DEBUG "Enable debug mode" OFF)

if(DEBUG)
    message(STATUS "Debug mode is ON")
    # Add debug-related flags or logic here
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG)
endif()

find_package(PostgreSQL REQUIRED)

add_executable(dfm_pattern_capture
    src/main.cpp
    ../shared/Geometry.cpp
    src/CommandLineArgs.cpp
    ../shared/LayoutFileReader.cpp
    src/GeometryProcessor.cpp
    src/DatabaseManager.cpp
    src/Utils.cpp
    src/DFMPatternCaptureApplication.cpp
)

target_link_libraries(dfm_pattern_capture PRIVATE PostgreSQL::PostgreSQL)
target_include_directories(dfm_pattern_capture PRIVATE 
    ${PostgreSQL_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/../shared
)
target_compile_options(dfm_pattern_capture PRIVATE -Wall -Wextra -O2)

add_executable(generate_test_gds 
    src/generate_test_gds.cpp
    ../shared/Geometry.cpp
)
target_include_directories(generate_test_gds PRIVATE 
    ${CMAKE_SOURCE_DIR}/../shared
)
target_compile_options(generate_test_gds PRIVATE -Wall -Wextra -O2)
