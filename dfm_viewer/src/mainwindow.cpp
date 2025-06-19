#include "mainwindow.h"
#include "Logging.h"
#include <QMenuBar>
#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QUiLoader>
#include <QFile>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    LOG_FUNCTION();
    settings = new QSettings("MyOrg", "DFMPatternViewer", this);

    // Load UI file
    QUiLoader loader;
    QFile file(":/mainwindow.ui");
    file.open(QFile::ReadOnly);
    QWidget *ui = loader.load(&file, this);
    file.close();
    setCentralWidget(ui);

    // Find widgets
    dbViewer = new DatabaseViewer(findChild<QWidget*>("dbViewerPlaceholder"));
    statusBar = findChild<QStatusBar*>("statusBar");
    fileMenu = findChild<QMenu*>("fileMenu");
    toolsMenu = findChild<QMenu*>("toolsMenu");
    exitAction = findChild<QAction*>("exitAction");
    batchPatternCaptureAction = findChild<QAction*>("batchPatternCaptureAction");

    gdsViewer = new GdsViewer();
    batchPatternCapture = nullptr;

    setWindowTitle("DFM Patterns Database Viewer");
    resize(1200, 800);

    // Connect signals
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);
    connect(batchPatternCaptureAction, &QAction::triggered, this, &MainWindow::openBatchPatternCapture);

    // Show GdsViewer by default
    gdsViewer->setAttribute(Qt::WA_DeleteOnClose);
    gdsViewer->setWindowTitle("GDS/OASIS Viewer");
    gdsViewer->resize(800, 600);
    gdsViewer->show();

    loadSettings();
    LOG_INFO("MainWindow initialized");
}

MainWindow::~MainWindow() {
    LOG_FUNCTION();
    saveSettings();
    delete settings;
    delete gdsViewer;
    delete batchPatternCapture;
    LOG_INFO("MainWindow destroyed");
}

void MainWindow::openBatchPatternCapture() {
    LOG_FUNCTION();
    if (!batchPatternCapture) {
        batchPatternCapture = new BatchPatternCapture();
        batchPatternCapture->setAttribute(Qt::WA_DeleteOnClose);
        batchPatternCapture->setWindowTitle("Batch PatternCapture");
        batchPatternCapture->resize(800, 600);
    }
    batchPatternCapture->show();
    batchPatternCapture->raise();
    LOG_INFO("Batch PatternCapture opened");
}

void MainWindow::loadSettings() {
    LOG_FUNCTION();
    if (settings->contains("geometry")) {
        restoreGeometry(settings->value("geometry").toByteArray());
    }
    LOG_DEBUG("Loaded MainWindow settings");
}

void MainWindow::saveSettings() {
    LOG_FUNCTION();
    settings->setValue("geometry", saveGeometry());
    settings->sync();
    LOG_DEBUG("Saved MainWindow settings");
}
