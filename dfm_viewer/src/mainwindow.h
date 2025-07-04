#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include "databaseviewer.h"
#include "gdsviewer.h"
#include "batchpatterncapture.h"

class NewDBInputDialog;
class QMenu;
class QAction;
class QStatusBar;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    NewDBInputDialog *dialog;

private slots:
    void openBatchPatternCapture();
    void connectToDatabase();
    void createNewDatabase(); // New slot for New Database action
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
    QAction *connectDbAction;
    QAction *newDbAction; // New action
    QAction *batchPatternCaptureAction;
    QStatusBar *statusBar;
};

#endif // MAINWINDOW_H
