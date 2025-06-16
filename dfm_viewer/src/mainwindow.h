#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include "databaseviewer.h"
#include "gdsviewer.h"
#include "patterncapture.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);

private:
    QSplitter *splitter;
    DatabaseViewer *dbViewer;
    GdsViewer *gdsViewer;
    PatternCapture *patternCapture;
};

#endif
