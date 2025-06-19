#include "databaseviewer.h"
#include "ZoomEventFilter.h"
#include "Logging.h"
#include "../shared/DatabaseManager.h"
#include <QStandardItemModel>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMessageBox>
#include <QInputDialog>
#include <QSplitter>

DatabaseViewer::DatabaseViewer(QWidget *parent) : QWidget(parent) {
    LOG_FUNCTION();
    settings = new QSettings("MyOrg", "DFMPatternViewer", this);
    dbManager = nullptr; // No initial database connection

    // Main layout with splitter
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    mainLayout->addWidget(splitter);

    // Left side: Database and Patterns lists
    QWidget *leftWidget = new QWidget(this);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
    databasesListLabel = new QLabel("Patterns Databases List", this);
    databasesList = new QListWidget(this);
    patternsListLabel = new QLabel("Patterns List for database: None", this);
    patternsList = new QListWidget(this);
    leftLayout->addWidget(databasesListLabel);
    leftLayout->addWidget(databasesList);
    leftLayout->addWidget(patternsListLabel);
    leftLayout->addWidget(patternsList);
    splitter->addWidget(leftWidget);

    // Right side: Tabbed panels
    QWidget *rightWidget = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
    refreshButton = new QPushButton("Refresh Patterns", this);
    dbConfigButton = new QPushButton("Configure Database", this);
    rightTabs = new QTabWidget(this);
    tableView = new QTableView(this);
    graphicsView = new QGraphicsView(this);
    scene = new QGraphicsScene(this);
    graphicsView->setScene(scene);
    graphicsView->setRenderHint(QPainter::Antialiasing);
    graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    rightTabs->addTab(graphicsView, "Geometries");
    rightTabs->addTab(tableView, "Metadata");
    rightLayout->addWidget(refreshButton);
    rightLayout->addWidget(dbConfigButton);
    rightLayout->addWidget(rightTabs);
    splitter->addWidget(rightWidget);

    splitter->setSizes({200, 600}); // Initial sizes for left and right
    graphicsView->installEventFilter(new ZoomEventFilter(graphicsView, this));

    // Connect signals
    connect(refreshButton, &QPushButton::clicked, this, &DatabaseViewer::refreshPatterns);
    connect(dbConfigButton, &QPushButton::clicked, this, &DatabaseViewer::configureDatabase);
    connect(databasesList, &QListWidget::itemClicked, this, &DatabaseViewer::onDatabaseSelected);
    connect(patternsList, &QListWidget::itemClicked, this, &DatabaseViewer::onPatternSelected);

    LOG_INFO("DatabaseViewer initialized");
}

DatabaseViewer::~DatabaseViewer() {
    LOG_FUNCTION();
    saveSettings();
    delete dbManager;
    delete settings;
    LOG_INFO("DatabaseViewer destroyed");
}

bool DatabaseViewer::connectToDatabase(const QString &dbName) {
    LOG_FUNCTION();
    patternsList->clear();
    patternsListLabel->setText("Patterns List for database: " + dbName);
    databasesList->clear();
    databasesList->addItem(dbName);
    scene->clear();
    tableView->setModel(nullptr);

    if (dbManager) {
        dbManager->disconnect();
        delete dbManager;
    }

    dbManager = new DatabaseManager(
        dbName.toStdString(),
        settings->value("dbUser", "").toString().toStdString(),
        settings->value("dbPassword", "").toString().toStdString(),
        settings->value("dbHost", "localhost").toString().toStdString(),
        settings->value("dbPort", "5432").toString().toStdString(),
        [this](const std::string& error) {
            QMessageBox::critical(this, "Database Error", QString::fromStdString(error));
        }
    );

    if (dbManager->connect() && dbManager->isValidSchema()) {
        dbManager->createTables();
        loadPatterns();
        LOG_INFO("Connected to database: " + dbName.toStdString());
        return true;
    } else {
        LOG_ERROR("Failed to connect to or validate database: " + dbName.toStdString());
        return false;
    }
}

void DatabaseViewer::configureDatabase() {
    LOG_FUNCTION();
    bool ok;
    QString dbName = QInputDialog::getText(this, "Database Configuration", "Database Name:",
                                           QLineEdit::Normal, settings->value("dbName", "").toString(), &ok);
    if (!ok) return;
    QString user = QInputDialog::getText(this, "Database Configuration", "Username:",
                                         QLineEdit::Normal, settings->value("dbUser", "").toString(), &ok);
    if (!ok) return;
    QString password = QInputDialog::getText(this, "Database Configuration", "Password:",
                                             QLineEdit::Password, settings->value("dbPassword", "").toString(), &ok);
    if (!ok) return;
    QString host = QInputDialog::getText(this, "Database Configuration", "Host:",
                                         QLineEdit::Normal, settings->value("dbHost", "localhost").toString(), &ok);
    if (!ok) return;
    QString port = QInputDialog::getText(this, "Database Configuration", "Port:",
                                         QLineEdit::Normal, settings->value("dbPort", "5432").toString(), &ok);
    if (!ok) return;

    settings->setValue("dbName", dbName);
    settings->setValue("dbUser", user);
    settings->setValue("dbPassword", password);
    settings->setValue("dbHost", host);
    settings->setValue("dbPort", port);
    settings->sync();
    LOG_INFO("Database configuration updated: dbName=" + dbName.toStdString());

    connectToDatabase(dbName);
}

void DatabaseViewer::refreshPatterns() {
    LOG_FUNCTION();
    scene->clear();
    patternsList->clear();
    loadPatterns();
}

void DatabaseViewer::onDatabaseSelected(QListWidgetItem *item) {
    LOG_FUNCTION();
    QString dbName = item->text();
    patternsListLabel->setText("Patterns List for database: " + dbName);
    connectToDatabase(dbName);
}

void DatabaseViewer::onPatternSelected(QListWidgetItem *item) {
    LOG_FUNCTION();
    int patternId = item->text().toInt();
    scene->clear();
    tableView->setModel(nullptr);

    if (!dbManager->isConnected() && !dbManager->connect()) {
        QMessageBox::critical(this, "Database Error", "Failed to connect to database.");
        LOG_ERROR("Failed to connect to database");
        return;
    }

    try {
        auto patterns = dbManager->getPatterns();
        auto geometries = dbManager->getGeometries(patternId);
        QStandardItemModel *model = new QStandardItemModel(this);
        model->setColumnCount(7);
        model->setHorizontalHeaderLabels({"ID", "Pattern Hash", "Mask Layer", "Input Layers", "Layout File", "Area", "Perimeter"});

        for (const auto& pattern : patterns) {
            if (pattern.id != patternId) continue;
            QList<QStandardItem*> items;
            items << new QStandardItem(QString::number(pattern.id));
            items << new QStandardItem(QString::fromStdString(pattern.pattern_hash));
            items << new QStandardItem(QString::number(pattern.mask_layer_number));
            items << new QStandardItem(QString::fromStdString(pattern.input_layers));
            items << new QStandardItem(QString::fromStdString(pattern.layout_file_name));

            double area = 0.0, perimeter = 0.0;
            for (const auto& geom : geometries) {
                if (geom.pattern_id == pattern.id) {
                    area = geom.area;
                    perimeter = geom.perimeter;
                    renderPolygon(QString::fromStdString(geom.coordinates));
                }
            }
            items << new QStandardItem(QString::number(area));
            items << new QStandardItem(QString::number(perimeter));
            model->appendRow(items);
        }
        tableView->setModel(model);
        graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
        LOG_INFO("Displayed pattern ID: " + std::to_string(patternId));
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Database Error", QString("Query failed: %1").arg(e.what()));
        LOG_ERROR("Database query failed: " + std::string(e.what()));
    }
}

void DatabaseViewer::loadDatabases() {
    LOG_FUNCTION();
    databasesList->clear();
    // No automatic database loading
}

void DatabaseViewer::loadPatterns() {
    LOG_FUNCTION();
    if (!dbManager || (!dbManager->isConnected() && !dbManager->connect())) {
        LOG_WARN("No database connection");
        return;
    }
    try {
        auto patterns = dbManager->getPatterns();
        patternsList->clear();
        for (const auto& pattern : patterns) {
            patternsList->addItem(QString::number(pattern.id));
        }
        LOG_INFO("Loaded " + std::to_string(patterns.size()) + " patterns from database");
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Database Error", QString("Query failed: %1").arg(e.what()));
        LOG_ERROR("Database query failed: " + std::string(e.what()));
    }
}

void DatabaseViewer::renderPolygon(const QString &coordinates) {
    LOG_FUNCTION();
    QJsonDocument doc = QJsonDocument::fromJson(coordinates.toUtf8());
    if (!doc.isArray()) {
        QMessageBox::critical(this, "Render Error", QString("Invalid JSON coordinates: %1").arg(coordinates));
        LOG_ERROR("Invalid JSON coordinates: " + coordinates.toStdString());
        return;
    }
    QPolygonF polygon;
    QJsonArray points = doc.array();
    for (const QJsonValue &point : points) {
        if (point.isArray()) {
            QJsonArray coords = point.toArray();
            if (coords.size() == 2) {
                polygon << QPointF(coords[0].toDouble(), coords[1].toDouble());
            }
        }
    }
    if (polygon.size() >= 3) {
        scene->addPolygon(polygon, QPen(Qt::black), QBrush(QColor(0, 0, 255, 100)));
        graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
        LOG_DEBUG("Rendered polygon with " + std::to_string(polygon.size()) + " points");
    } else {
        LOG_WARN("Polygon not rendered: insufficient points (" + std::to_string(polygon.size()) + ")");
    }
}

void DatabaseViewer::loadSettings() {
    LOG_FUNCTION();
    // Handled by QSettings in constructor
}

void DatabaseViewer::saveSettings() {
    LOG_FUNCTION();
    settings->sync();
}
