cmake_minimum_required(VERSION 3.16)
project(DFMPatternViewer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Gui Widgets Sql UiTools REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPQXX REQUIRED libpqxx)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/databaseviewer.cpp
    src/gdsviewer.cpp
    src/batchpatterncapture.cpp
    ../shared/Geometry.cpp
    ../shared/LayoutFileReader.cpp
    ../shared/Logging.cpp
    ../shared/DatabaseManager.cpp
)

set(HEADERS
    src/mainwindow.h
    src/databaseviewer.h
    src/gdsviewer.h
    src/batchpatterncapture.h
    src/ZoomEventFilter.h
    ../shared/Geometry.h
    ../shared/LayoutFileReader.h
    ../shared/Logging.h
    ../shared/DatabaseManager.h
)

set(RESOURCES
    resources.qrc
)

set(UI_FILES
    src/mainwindow.ui
    src/batchpatterncapture.ui
)

add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
    ${UI_FILES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/../shared
    ${LIBPQXX_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Sql
    Qt5::UiTools
    ${LIBPQXX_LIBRARIES}
    -lpq
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    ${LIBPQXX_CFLAGS_OTHER}
)
