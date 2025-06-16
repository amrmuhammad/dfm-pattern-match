cmake_minimum_required(VERSION 3.12)
project(DFMPatternCapture)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpq)

# Add executable
add_executable(dfm_pattern_capture src/main.cpp)

# Link libraries
target_link_libraries(dfm_pattern_capture ${PQXX_LIBRARIES})
target_include_directories(dfm_pattern_capture PRIVATE ${PQXX_INCLUDE_DIRS})
target_compile_options(dfm_pattern_capture PRIVATE ${PQXX_CFLAGS_OTHER})

# Set compiler flags
target_compile_options(dfm_pattern_capture PRIVATE -Wall -Wextra -O2)
