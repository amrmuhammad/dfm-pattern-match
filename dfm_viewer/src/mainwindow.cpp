#include "mainwindow.h"
#include "Logging.h"
#include <QMenuBar>
#include <QAction>
#include <QApplication>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include "newDBinputdialog.h"
#include <QWidget>
#include "connectdbdialog.h"
#include "../shared/DatabaseManager.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    LOG_FUNCTION();
    
    dialog = nullptr;
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
    newDbAction = new QAction("New Database", this);
    connectDbAction = new QAction("Connect to DB", this);
    exitAction = new QAction("Exit", this);
    fileMenu->addAction(newDbAction);
    fileMenu->addAction(connectDbAction);
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
    gdsViewer = new GdsViewer(this);
    batchPatternCapture = nullptr;

    setWindowTitle("DFM Patterns Database Viewer");
    resize(1200, 800);

    // Connect signals
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);
    connect(newDbAction, &QAction::triggered, this, &MainWindow::createNewDatabase);
    connect(connectDbAction, &QAction::triggered, this, &MainWindow::connectToDatabase);
    connect(batchPatternCaptureAction, &QAction::triggered, this, &MainWindow::openBatchPatternCapture);
    connect(this, &QObject::destroyed, this, [this]() { if (dialog) dialog->deleteLater(); });

    // Show GdsViewer maximized and ensure MainWindow is in front
    gdsViewer->setAttribute(Qt::WA_DeleteOnClose);
    gdsViewer->setWindowTitle("GDS/OASIS Viewer");
    gdsViewer->showMaximized();
    gdsViewer->resize(800, 600); // Fallback size
    this->show();
    this->raise();
    this->activateWindow();

    loadSettings();
    LOG_INFO("MainWindow initialized");
}

MainWindow::~MainWindow() {
    LOG_FUNCTION();
    saveSettings();
    dialog = nullptr; // dialog deleted via deleteLater
    delete settings;
    delete gdsViewer;
    delete batchPatternCapture;
    LOG_INFO("MainWindow destroyed");
}

void MainWindow::openBatchPatternCapture() {
    LOG_FUNCTION();
    if (!batchPatternCapture) {
        batchPatternCapture = new BatchPatternCapture(this);
        batchPatternCapture->setAttribute(Qt::WA_DeleteOnClose);
        batchPatternCapture->setWindowTitle("Batch Pattern Capture");
        batchPatternCapture->resize(800, 600);
    }
    batchPatternCapture->show();
    batchPatternCapture->raise();
    LOG_INFO("Batch Pattern Capture opened");
}

void MainWindow::createNewDatabase() {
    LOG_FUNCTION();
    
    if (dialog) {
    	dialog->raise();
    	return;
    }
    
    dialog = new NewDBInputDialog("New Database", "Enter database name:", this);
    while (true) {
        if (dialog->exec() != QDialog::Accepted) {
            LOG_INFO("New database creation cancelled");
            dialog->deleteLater();
            dialog = nullptr;
            return;
        }
        QString dbName = dialog->getText();

        // Retrieve database settings
        QString user = settings->value("dbUser", "").toString();
        QString password = settings->value("dbPassword", "").toString();
        QString host = settings->value("dbHost", "localhost").toString();
        QString port = settings->value("dbPort", "5432").toString();

        // Create DatabaseManager instance
        DatabaseManager dbManager(
            dbName.toStdString(),
            user.toStdString(),
            password.toStdString(),
            host.toStdString(),
            port.toStdString(),
            [this](const std::string& error) {
                QMessageBox::critical(this, "Database Error", QString::fromStdString(error));
            }
        );

        // Create the database
        CreateDatabaseIfNotExistsStatusCode status_code = dbManager.createDatabaseIfNotExists();
        if (status_code == CreateDatabaseIfNotExistsStatusCode::DB_EXISTS) {
            dialog->setValidationMessage(
                QString("Database '%1' already exists. Use File → Connect to DB.").arg(dbName),
                false
            );
            LOG_WARN("Database already exists: " + dbName.toStdString());
            continue;
        }

        // Connect to the new database
        if (!dbManager.connect()) {
            dialog->setValidationMessage(
                QString("Failed to connect to new database '%1'. Try again.").arg(dbName),
                false
            );
            LOG_ERROR("Failed to connect to new database: " + dbName.toStdString());
            continue;
        }

        // Create tables
        if (!dbManager.createTables()) {
            dialog->setValidationMessage(
                QString("Failed to create tables in database '%1'. Try again.").arg(dbName),
                false
            );
            LOG_ERROR("Failed to create tables in database: " + dbName.toStdString());
            continue;
        }

        // Initialize patterns_db_version with valid GUID
        if (!dbManager.initializeVersionGUID()) {
            dialog->setValidationMessage(
                QString("Failed to initialize database version for '%1'. Try again.").arg(dbName),
                false
            );
            LOG_ERROR("Failed to initialize patterns_db_version for database: " + dbName.toStdString());
            continue;
        }

        // Connect DatabaseViewer to the new database
        if (dbViewer->connectToDatabase(dbName)) {
            QMessageBox::information(this, "Success", "New database '" + dbName + "' created and connected successfully.");
            LOG_INFO("Successfully created and connected to new database: " + dbName.toStdString());
            dialog->deleteLater();
            dialog = nullptr;
            break;
        } else {
            dialog->setValidationMessage(
                QString("Failed to connect DatabaseViewer to '%1'. Try again.").arg(dbName),
                false
            );
            LOG_ERROR("Failed to connect DatabaseViewer to new database: " + dbName.toStdString());
            continue;
        }
    }
}

void MainWindow::connectToDatabase() {
    LOG_FUNCTION();
    ConnectDbDialog dialog(this, dbViewer);
    dialog.exec();
    LOG_INFO("Connect to Database dialog closed");
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
