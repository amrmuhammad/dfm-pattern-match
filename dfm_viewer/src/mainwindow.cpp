#include "mainwindow.h"
#include "Logging.h"
#include <QMenuBar>
#include <QAction>
#include <QApplication>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    LOG_FUNCTION();
    settings = new QSettings("MyOrg", "DFMPatternViewer", this);

    // Create central widget and layout
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *centralLayout = new QVBoxLayout(centralWidget);
    setCentralWidget(centralWidget);

    // Create placeholder for DatabaseViewer
    QWidget *dbViewerPlaceholder = new QWidget(centralWidget);
    centralLayout->addWidget(dbViewerPlaceholder);

    // Instantiate DatabaseViewer
    dbViewer = new DatabaseViewer(dbViewerPlaceholder);
    dbViewer->setParent(dbViewerPlaceholder);

    // Create menu bar
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    // File menu
    fileMenu = new QMenu("File", this);
    exitAction = new QAction("Exit", this);
    fileMenu->addAction(exitAction);
    menuBar->addMenu(fileMenu);

    // Tools menu
    toolsMenu = new QMenu("Tools", this);
    batchPatternCaptureAction = new QAction("Batch Pattern Capture", this);
    toolsMenu->addAction(batchPatternCaptureAction);
    menuBar->addMenu(toolsMenu);

    // Status bar
    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);

    // Initialize other components
    gdsViewer = new GdsViewer();
    batchPatternCapture = nullptr;

    setWindowTitle("DFM Patterns Database Viewer");
    resize(1200, 800);

    // Connect signals
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);
    connect(batchPatternCaptureAction, &QAction::triggered, this, &MainWindow::openBatchPatternCapture);

    // Show GdsViewer by default (version 0.13)
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
