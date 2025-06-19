#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include "databaseviewer.h"
#include "gdsviewer.h"
#include "batchpatterncapture.h"

class QMenu;
class QAction;
class QStatusBar;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openBatchPatternCapture();
    void loadSettings();
    void saveSettings();

private:
    QSettings *settings;
    DatabaseViewer *dbViewer;
    GdsViewer *gdsViewer;
    BatchPatternCapture *batchPatternCapture;
    QMenu *fileMenu;
    QMenu *toolsMenu;
    QAction *exitAction;
    QAction *batchPatternCaptureAction;
    QStatusBar *statusBar;
};

#endif // MAINWINDOW_H
