cmake_minimum_required(VERSION 3.12)
project(DFMPatternCapture)

# Set policy CMP0167 to OLD to use FindBoost (suppress deprecation warning)
cmake_policy(SET CMP0167 OLD)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Print build type
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Option for debug mode
option(DEBUG "Enable debug mode" OFF)
if(DEBUG)
    message(STATUS "Debug mode is ON")
endif()

# Add DEBUG definition for Debug builds
if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_definitions(DEBUG)
endif()

# Find Boost (no components needed for header-only Geometry)
find_package(Boost 1.65 REQUIRED)
if(Boost_FOUND)
    message(STATUS "Boost found: ${Boost_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "Boost not found")
endif()

# Find libpqxx
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)
if(PQXX_FOUND)
    message(STATUS "libpqxx found: ${PQXX_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "libpqxx not found")
endif()

# Define source files for dfm_pattern_capture
set(DFM_PATTERN_CAPTURE_SOURCES
    src/main.cpp
    src/CommandLineArgs.cpp
    src/GeometryProcessor.cpp
    src/Utils.cpp
    src/DFMPatternCaptureApplication.cpp
    ../shared/Geometry.cpp
    ../shared/LayoutFileReader.cpp
    ../shared/Logging.cpp
    ../shared/DatabaseManager.cpp
)

# Define source files for generate_test_gds
set(GENERATE_TEST_GDS_SOURCES
    src/generate_test_gds.cpp
    ../shared/Geometry.cpp
    ../shared/Logging.cpp
)

# Create dfm_pattern_capture executable
add_executable(dfm_pattern_capture ${DFM_PATTERN_CAPTURE_SOURCES})
target_include_directories(dfm_pattern_capture PRIVATE
    ${Boost_INCLUDE_DIRS}
    ${PQXX_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/../shared
)
target_link_libraries(dfm_pattern_capture PRIVATE
    Boost::headers
    ${PQXX_LIBRARIES}
)
target_compile_options(dfm_pattern_capture PRIVATE
    -Wall -Wextra -O2
)

# Create generate_test_gds executable
add_executable(generate_test_gds ${GENERATE_TEST_GDS_SOURCES})
target_include_directories(generate_test_gds PRIVATE
    ${CMAKE_SOURCE_DIR}/../shared
)
target_compile_options(generate_test_gds PRIVATE
    -Wall -Wextra -O2
)
