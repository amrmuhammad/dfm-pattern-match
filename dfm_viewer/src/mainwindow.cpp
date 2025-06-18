#include "mainwindow.h"
#include "Logging.h"
#include <QMenuBar>
#include <QAction>
#include <QApplication>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    LOG_FUNCTION();
    settings = new QSettings("MyOrg", "DFMPatternViewer", this);
    splitter = new QSplitter(Qt::Horizontal, this);
    dbViewer = new DatabaseViewer(this);
    gdsViewer = nullptr; // Created on demand
    batchPatternCapture = nullptr; // Created on demand
    statusBar = QMainWindow::statusBar();

    splitter->addWidget(dbViewer);
    setCentralWidget(splitter);
    setWindowTitle("DFM Patterns Database Viewer");
    resize(1200, 800);

    setupMenuBar();
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

void MainWindow::setupMenuBar() {
    QMenuBar *menuBar = this->menuBar();
    fileMenu = menuBar->addMenu("File");
    exitAction = fileMenu->addAction("Exit");
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);

    toolsMenu = menuBar->addMenu("Tools");
    gdsViewerAction = toolsMenu->addAction("GDS/OASIS Viewer");
    batchPatternCaptureAction = toolsMenu->addAction("Batch PatternCapture");
    connect(gdsViewerAction, &QAction::triggered, this, &MainWindow::openGdsViewer);
    connect(batchPatternCaptureAction, &QAction::triggered, this, &MainWindow::openBatchPatternCapture);
}

void MainWindow::openGdsViewer() {
    LOG_FUNCTION();
    if (!gdsViewer) {
        gdsViewer = new GdsViewer();
        gdsViewer->setAttribute(Qt::WA_DeleteOnClose); // Delete when closed
        gdsViewer->setWindowTitle("GDS/OASIS Viewer");
        gdsViewer->resize(800, 600);
    }
    gdsViewer->show();
    gdsViewer->raise();
    LOG_INFO("GDS/OASIS Viewer opened");
}

void MainWindow::openBatchPatternCapture() {
    LOG_FUNCTION();
    if (!batchPatternCapture) {
        batchPatternCapture = new BatchPatternCapture();
        batchPatternCapture->setAttribute(Qt::WA_DeleteOnClose); // Delete when closed
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
    if (settings->contains("splitterState")) {
        splitter->restoreState(settings->value("splitterState").toByteArray());
    }
    LOG_DEBUG("Loaded MainWindow settings");
}

void MainWindow::saveSettings() {
    LOG_FUNCTION();
    settings->setValue("geometry", saveGeometry());
    settings->setValue("splitterState", splitter->saveState());
    settings->sync();
    LOG_DEBUG("Saved MainWindow settings");
}
