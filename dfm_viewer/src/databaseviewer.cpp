#include "databaseviewer.h"
#include <QStandardItemModel>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>

DatabaseViewer::DatabaseViewer(QWidget *parent) : QWidget(parent) {
    try {
        conn = new pqxx::connection("dbname=test_db user=your_username password=your_password host=localhost port=5432");
    } catch (const std::exception &e) {
        qCritical() << "Database connection failed:" << e.what();
        return;
    }

    QVBoxLayout *layout = new QVBoxLayout(this);
    refreshButton = new QPushButton("Refresh Patterns", this);
    tableView = new QTableView(this);
    graphicsView = new QGraphicsView(this);
    scene = new QGraphicsScene(this);
    graphicsView->setScene(scene);
    layout->addWidget(refreshButton);
    layout->addWidget(tableView);
    layout->addWidget(graphicsView);

    connect(refreshButton, &QPushButton::clicked, this, &DatabaseViewer::refreshPatterns);

    loadPatterns();
}

void DatabaseViewer::refreshPatterns() {
    scene->clear();
    loadPatterns();
}

void DatabaseViewer::loadPatterns() {
    try {
        pqxx::work txn(*conn);
        pqxx::result patterns = txn.exec("SELECT id, pattern_hash, mask_layer_number, input_layers_numbers, layout_file_name FROM patterns");
        pqxx::result geometries = txn.exec("SELECT pattern_id, layer_number, geometry_type, coordinates, area, perimeter FROM pattern_geometries");

        QStandardItemModel *model = new QStandardItemModel(this);
        model->setColumnCount(7);
        model->setHorizontalHeaderLabels({"ID", "Pattern Hash", "Mask Layer", "Input Layers", "Layout File", "Area", "Perimeter"});
        for (const auto &row : patterns) {
            QList<QStandardItem*> items;
            items << new QStandardItem(QString::number(row["id"].as<int>()));
            items << new QStandardItem(row["pattern_hash"].c_str());
            items << new QStandardItem(QString::number(row["mask_layer_number"].as<int>()));
            items << new QStandardItem(row["input_layers_numbers"].c_str());
            items << new QStandardItem(row["layout_file_name"].c_str());

            for (const auto &geom : geometries) {
                if (geom["pattern_id"].as<int>() == row["id"].as<int>()) {
                    items << new QStandardItem(QString::number(geom["area"].as<double>()));
                    items << new QStandardItem(QString::number(geom["perimeter"].as<double>()));
                    renderPolygon(geom["coordinates"].c_str());
                    break;
                }
            }
            model->appendRow(items);
        }
        tableView->setModel(model);
        txn.commit();
    } catch (const std::exception &e) {
        qCritical() << "Database query failed:" << e.what();
    }
}

void DatabaseViewer::renderPolygon(const QString &coordinates) {
    QJsonDocument doc = QJsonDocument::fromJson(coordinates.toUtf8());
    if (!doc.isArray()) {
        qCritical() << "Invalid JSON coordinates:" << coordinates;
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
        scene->addPolygon(polygon, QPen(Qt::black), QBrush(Qt::blue));
        graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    }
}
