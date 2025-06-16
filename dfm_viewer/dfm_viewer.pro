QT += core gui widgets
CONFIG += c++17
SOURCES += src/main.cpp src/mainwindow.cpp src/databaseviewer.cpp src/gdsviewer.cpp src/patterncapture.cpp \
           ../shared/LayoutFileReader.cpp \
           ../shared/Geometry.cpp
HEADERS += src/mainwindow.h src/databaseviewer.h src/gdsviewer.h src/patterncapture.h \
           ../shared/LayoutFileReader.h \
           ../shared/Geometry.h
LIBS += -lpqxx -lpq
INCLUDEPATH += /usr/include /usr/include/postgresql /home/amrmuhammad/dev/dfm_pattern_match4/shared
