#include "gdsviewer.h"
#include <QDebug>

GdsViewer::GdsViewer(QWidget *parent) : QWidget(parent), reader(nullptr) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    loadButton = new QPushButton("Load GDSII/OASIS File", this);
    layerCombo = new QComboBox(this);
    graphicsView = new QGraphicsView(this);
    scene = new QGraphicsScene(this);
    graphicsView->setScene(scene);
    layout->addWidget(loadButton);
    layout->addWidget(layerCombo);
    layout->addWidget(graphicsView);

    connect(loadButton, &QPushButton::clicked, this, &GdsViewer::loadGdsFile);
    connect(layerCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GdsViewer::updateLayer);
}

void GdsViewer::loadGdsFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open GDSII/OASIS File", "", "GDSII/OASIS Files (*.gds *.oas)");
    if (fileName.isEmpty()) return;

    try {
        scene->clear();
        layerCombo->clear();
        if (reader) delete reader;
        reader = new LayoutFileReader(fileName.toStdString());
        availableLayers = reader->getAvailableLayers();
        for (int layer : availableLayers) {
            layerCombo->addItem(QString("Layer %1").arg(layer));
        }
        if (!availableLayers.empty()) {
            updateLayer(0);
        } else {
            qCritical() << "No layers found in file:" << fileName;
        }
    } catch (const std::exception &e) {
        qCritical() << "Failed to load GDSII/OASIS file:" << e.what();
    }
}

void GdsViewer::updateLayer(int index) {
    if (!reader || index < 0 || index >= static_cast<int>(availableLayers.size())) return;
    try {
        scene->clear();
        Layer layer = reader->loadLayer(availableLayers[index]);
        renderLayer(layer);
    } catch (const std::exception &e) {
        qCritical() << "Failed to load layer:" << e.what();
    }
}

void GdsViewer::renderLayer(const Layer &layer) {
    for (const auto &poly : layer.polygons) {
        QPolygonF qpoly;
        for (const auto &pt : poly.points) {
            qpoly << QPointF(pt.x, pt.y);
        }
        scene->addPolygon(qpoly, QPen(Qt::black), QBrush(QColor(0, 0, 255, 100)));
    }
    graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
}
