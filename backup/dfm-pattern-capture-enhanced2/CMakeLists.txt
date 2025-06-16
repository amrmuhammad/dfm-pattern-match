cmake_minimum_required(VERSION 3.12)
project(DFMPatternCapture)

# Set C++ standard for all targets
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages for dfm_pattern_capture
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpq)

# Add executable for DFM Pattern Capture Application
add_executable(dfm_pattern_capture
    src/main.cpp
    src/Geometry.cpp
    src/CommandLineArgs.cpp
    src/LayoutFileReader.cpp
    src/GeometryProcessor.cpp
    src/DatabaseManager.cpp
    src/Utils.cpp
    src/DFMPatternCaptureApplication.cpp
)

# Link libraries and set options for dfm_pattern_capture
target_link_libraries(dfm_pattern_capture ${PQXX_LIBRARIES})
target_include_directories(dfm_pattern_capture PRIVATE ${PQXX_INCLUDE_DIRS})
target_compile_options(dfm_pattern_capture PRIVATE ${PQXX_CFLAGS_OTHER} -Wall -Wextra -O2)

# Add executable for Generate Test GDS
add_executable(generate_test_gds src/generate_test_gds.cpp)

# Set compiler flags for generate_test_gds
target_compile_options(generate_test_gds PRIVATE -Wall -Wextra -O2)
