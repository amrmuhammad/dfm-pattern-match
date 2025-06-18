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
    gdsViewer = new GdsViewer(this);
    patternCapture = new PatternCapture(this);
    statusBar = QMainWindow::statusBar();

    splitter->addWidget(dbViewer);
    splitter->addWidget(gdsViewer);
    splitter->addWidget(patternCapture);

    setCentralWidget(splitter);
    setWindowTitle("DFM Patterns Database Viewer");
    resize(1200, 800);

    QMenuBar *menuBar = this->menuBar();
    QMenu *fileMenu = menuBar->addMenu("File");
    QAction *exitAction = fileMenu->addAction("Exit");
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);

    loadSettings();
    LOG_INFO("MainWindow initialized");
}

MainWindow::~MainWindow() {
    LOG_FUNCTION();
    saveSettings();
    delete settings;
    LOG_INFO("MainWindow destroyed");
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
