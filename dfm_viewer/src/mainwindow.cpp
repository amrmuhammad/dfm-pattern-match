#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    splitter = new QSplitter(Qt::Horizontal, this);
    dbViewer = new DatabaseViewer(this);
    gdsViewer = new GdsViewer(this);
    patternCapture = new PatternCapture(this);

    splitter->addWidget(dbViewer);
    splitter->addWidget(gdsViewer);
    splitter->addWidget(patternCapture);

    setCentralWidget(splitter);
    setWindowTitle("DFM Patterns Database Viewer");
    resize(1200, 800);
}
