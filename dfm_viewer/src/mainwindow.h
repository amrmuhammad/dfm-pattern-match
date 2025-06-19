#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStatusBar>
#include <QSettings>
#include <QMenu>
#include <QAction>
#include "databaseviewer.h"
#include "gdsviewer.h"
#include "batchpatterncapture.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openBatchPatternCapture();

private:
    void saveSettings();
    void loadSettings();
    void setupMenuBar();

    DatabaseViewer *dbViewer;
    GdsViewer *gdsViewer;
    BatchPatternCapture *batchPatternCapture;
    QStatusBar *statusBar;
    QSettings *settings;
    QMenu *fileMenu;
    QMenu *toolsMenu;
    QAction *exitAction;
    QAction *batchPatternCaptureAction;
};

#endif
