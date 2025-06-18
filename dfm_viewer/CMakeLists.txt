cmake_minimum_required(VERSION 3.16)
project(DFMPatternViewer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets Core REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

include_directories(${PQXX_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../shared)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/databaseviewer.cpp
    src/gdsviewer.cpp
    src/patterncapture.cpp
    ../shared/Geometry.cpp
    ../shared/LayoutFileReader.cpp
    ../shared/Logging.cpp
    ../shared/DatabaseManager.cpp
)

set(HEADERS
    src/mainwindow.h
    src/databaseviewer.h
    src/gdsviewer.h
    src/patterncapture.h
    src/ZoomEventFilter.h
    ../shared/Geometry.h
    ../shared/LayoutFileReader.h
    ../shared/Logging.h
    ../shared/DatabaseManager.h
)

add_executable(DFMPatternViewer ${SOURCES} ${HEADERS})

target_link_libraries(DFMPatternViewer PRIVATE
    Qt5::Widgets
    Qt5::Core
    ${PQXX_LIBRARIES}
)

install(TARGETS DFMPatternViewer DESTINATION bin)
