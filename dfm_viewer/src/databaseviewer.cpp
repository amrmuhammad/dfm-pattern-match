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

DatabaseViewer::DatabaseViewer(QWidget *parent) : QWidget(parent) {
    LOG_FUNCTION();
    settings = new QSettings("MyOrg", "DFMPatternViewer", this);
    db_manager = new DatabaseManager("test_db", settings->value("dbUser", "").toString().toStdString(),
                                     settings->value("dbPassword", "").toString().toStdString(),
                                     settings->value("dbHost", "localhost").toString().toStdString(),
                                     settings->value("dbPort", "5432").toString().toStdString(),
                                     [this](const std::string& error) {
                                         QMessageBox::critical(this, "Database Error", QString::fromStdString(error));
                                     });

    QVBoxLayout *layout = new QVBoxLayout(this);
    refreshButton = new QPushButton("Refresh Patterns", this);
    dbConfigButton = new QPushButton("Configure Database", this);
    tableView = new QTableView(this);
    graphicsView = new QGraphicsView(this);
    scene = new QGraphicsScene(this);
    graphicsView->setScene(scene);
    graphicsView->setRenderHint(QPainter::Antialiasing);
    graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    layout->addWidget(refreshButton);
    layout->addWidget(dbConfigButton);
    layout->addWidget(tableView);
    layout->addWidget(graphicsView);

    graphicsView->installEventFilter(new ZoomEventFilter(graphicsView, this));

    connect(refreshButton, &QPushButton::clicked, this, &DatabaseViewer::refreshPatterns);
    connect(dbConfigButton, &QPushButton::clicked, this, &DatabaseViewer::configureDatabase);

    if (db_manager->isConnected() || db_manager->connect()) {
        db_manager->createTables();
        loadPatterns();
    }
    LOG_INFO("DatabaseViewer initialized");
}

void DatabaseViewer::configureDatabase() {
    LOG_FUNCTION();
    bool ok;
    QString dbName = QInputDialog::getText(this, "Database Configuration", "Database Name:",
                                           QLineEdit::Normal, settings->value("dbName", "test_db").toString(), &ok);
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
    LOG_INFO("Database configuration updated: dbName=" + dbName.toStdString() + ", host=" + host.toStdString());

    db_manager->disconnect();
    delete db_manager;
    db_manager = new DatabaseManager(dbName.toStdString(), user.toStdString(), password.toStdString(),
                                     host.toStdString(), port.toStdString(),
                                     [this](const std::string& error) {
                                         QMessageBox::critical(this, "Database Error", QString::fromStdString(error));
                                     });
    if (db_manager->isConnected() || db_manager->connect()) {
        db_manager->createTables();
        refreshPatterns();
    }
}

void DatabaseViewer::refreshPatterns() {
    LOG_FUNCTION();
    scene->clear();
    loadPatterns();
}

void DatabaseViewer::loadPatterns() {
    LOG_FUNCTION();
    if (!db_manager->isConnected() && !db_manager->connect()) {
        LOG_WARN("No database connection");
        return;
    }
    try {
        auto patterns = db_manager->getPatterns();
        auto geometries = db_manager->getGeometries();
        QStandardItemModel *model = new QStandardItemModel(this);
        model->setColumnCount(7);
        model->setHorizontalHeaderLabels({"ID", "Pattern Hash", "Mask Layer", "Input Layers", "Layout File", "Area", "Perimeter"});

        for (const auto& pattern : patterns) {
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
    // Handled by QSettings in constructor
}

void DatabaseViewer::saveSettings() {
    LOG_FUNCTION();
    settings->sync();
}
