QT += core gui widgets
CONFIG += c++17

TARGET = DFMPatternViewer
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/databaseviewer.cpp \
    src/gdsviewer.cpp \
-   src/patterncapture.cpp \
+   src/batchpatterncapture.cpp \
    ../shared/Geometry.cpp \
    ../shared/LayoutFileReader.cpp \
    ../shared/Logging.cpp \
    ../shared/DatabaseManager.cpp \
    src/connectdbdialog.cpp

HEADERS += \
    src/mainwindow.h \
    src/databaseviewer.h \
    src/gdsviewer.h \
-   src/patterncapture.h \
+   src/batchpatterncapture.h \
    src/ZoomEventFilter.h \
    ../shared/Geometry.h \
    ../shared/LayoutFileReader.h \
    ../shared/Logging.h \
    ../shared/DatabaseManager.h \
    src/connectdbdialog.h

INCLUDEPATH += \
    $$PWD/../shared \
    /usr/include \
    /usr/include/postgresql

LIBS += -L/usr/lib -lpqxx -lpq

DEPENDPATH += $$PWD/src $$PWD/../shared
