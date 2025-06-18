#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QStatusBar>
#include "databaseviewer.h"
#include "gdsviewer.h"
#include "patterncapture.h"
#include <QSettings>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void saveSettings();
    void loadSettings();

    QSplitter *splitter;
    DatabaseViewer *dbViewer;
    GdsViewer *gdsViewer;
    PatternCapture *patternCapture;
    QStatusBar *statusBar;
    QSettings *settings;
};

#endif
