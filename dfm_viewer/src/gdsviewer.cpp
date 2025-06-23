#include "gdsviewer.h"
#include "ZoomEventFilter.h"
#include "Logging.h"
#include <QDebug>
#include <QMessageBox>
#include <QCoreApplication>
#include <QThread>
#include <sstream>

//////////////////////////////////////////////////////////////////////////////////////////////////
void logQPolygonF(const QPolygonF& poly, const std::string& context = "") {
    std::ostringstream oss;
    oss << (context.empty() ? "QPolygonF" : context) << ": [";
    for (int i = 0; i < poly.size(); ++i) {
        const QPointF& point = poly[i];
        oss << "(" << point.x() << ", " << point.y() << ")";
        if (i < poly.size() - 1) oss << ", ";
    }
    oss << "]";
    LOG_DEBUG(oss.str());
}
//////////////////////////////////////////////////////////////////////////////////////////////////

GdsViewer::GdsViewer(QWidget *parent) : QMainWindow(parent), reader(nullptr) {
    LOG_FUNCTION();
    centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    loadButton = new QPushButton("Load GDSII/OASIS File", this);
    layerCombo = new QComboBox(this);
    
    graphicsView = new QGraphicsView(this);
    graphicsView->setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing, true); // Prevent bounding box artifacts
    
    scene = new QGraphicsScene(this);
    graphicsView->setScene(scene);
    graphicsView->setRenderHint(QPainter::Antialiasing);
    graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    progressDialog = new QProgressDialog("Loading file...", "Cancel", 0, 100, this);
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setMinimumDuration(0);
    progressDialog->hide();
    layout->addWidget(loadButton);
    layout->addWidget(layerCombo);
    layout->addWidget(graphicsView);

    setCentralWidget(centralWidget);
    graphicsView->installEventFilter(new ZoomEventFilter(graphicsView, this));

    connect(loadButton, &QPushButton::clicked, this, &GdsViewer::loadGdsFile);
    connect(layerCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GdsViewer::updateLayer);

    LOG_INFO("GdsViewer initialized");
}

void GdsViewer::loadGdsFile() {
    LOG_FUNCTION();
    QString fileName = QFileDialog::getOpenFileName(this, "Open GDSII/OASIS File", "", "GDSII/OASIS Files (*.gds *.oas)");
    if (fileName.isEmpty()) return;

    try {
        progressDialog->setValue(0);
        progressDialog->show();
        scene->clear();
        layerCombo->clear();
        if (reader) delete reader;
        reader = new LayoutFileReader(fileName.toStdString());
        for (int i = 0; i <= 100; i += 10) {
            progressDialog->setValue(i);
            QCoreApplication::processEvents();
            QThread::msleep(100);
        }
        availableLayers = reader->getAvailableLayersAndDatatypes();
        for (const auto& [layer, datatype] : availableLayers) {
            layerCombo->addItem(QString("Layer %1:%2").arg(layer).arg(datatype));
        }
        if (!availableLayers.empty()) {
            updateLayer(0);
            LOG_INFO("Loaded file: " + fileName.toStdString() + " with " + std::to_string(availableLayers.size()) + " layers");
        } else {
            QMessageBox::warning(this, "Warning", "No layers found in file: " + fileName);
            LOG_WARN("No layers found in file: " + fileName.toStdString());
        }
        progressDialog->hide();
    } catch (const std::exception &e) {
        progressDialog->hide();
        QMessageBox::critical(this, "Error", QString("Failed to load GDSII/OASIS file: %1").arg(e.what()));
        LOG_ERROR("Failed to load GDSII/OASIS file: " + std::string(e.what()));
    }
}

void GdsViewer::updateLayer(int index) {
    LOG_FUNCTION();
    if (!reader || index < 0 || index >= static_cast<int>(availableLayers.size())) return;
    try {
        scene->clear();
        Layer layer = reader->loadLayer(availableLayers[index].first, availableLayers[index].second);
        renderLayer(layer);
        LOG_INFO("Rendered layer " + std::to_string(availableLayers[index].first) + ":" + std::to_string(availableLayers[index].second) +
                 " with " + std::to_string(layer.getPolygonCount()) + " polygons");
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Error", QString("Failed to load layer: %1").arg(e.what()));
        LOG_ERROR("Failed to load layer: " + std::string(e.what()));
    }
}

void GdsViewer::renderLayer(const Layer &layer) {
    LOG_FUNCTION();
    
    //bool display_only_one_polygon = true;
    
    // for debugging  TODO:remove after debugging////
    //Layer layer_debug(66, 20);
    //Polygon test_poly;
    //test_poly.points = {{1000, 2000}, {2000, 2000}, {2000, 3000}, {1000, 3000}}; 
    //layer_debug.polygons = {test_poly};
    ///////////////////////////////////////////////
    
    for (const auto &poly : layer.polygons) {
        QPolygonF qpoly;
        for (const auto &pt : poly.points) {
            qpoly << QPointF(pt.x, pt.y);
        }
        
        // logging
        logQPolygonF(qpoly, "GdsViewer::renderLayer");
        
        scene->addPolygon(qpoly, QPen(Qt::black), QBrush(QColor(0, 0, 255, 100)));
        
        //if (display_only_one_polygon) break; // exit for loop
    }
    graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    LOG_DEBUG("Rendered " + std::to_string(layer.polygons.size()) + " polygons");
}
