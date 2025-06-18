#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
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
    void openGdsViewer();
    void openBatchPatternCapture();

private:
    void saveSettings();
    void loadSettings();
    void setupMenuBar();

    QSplitter *splitter;
    DatabaseViewer *dbViewer;
    GdsViewer *gdsViewer; // Owned, but not in splitter
    BatchPatternCapture *batchPatternCapture; // Owned, but not in splitter
    QStatusBar *statusBar;
    QSettings *settings;
    QMenu *fileMenu;
    QMenu *toolsMenu;
    QAction *exitAction;
    QAction *gdsViewerAction;
    QAction *batchPatternCaptureAction;
};

#endif
