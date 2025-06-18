#ifndef GDSVIEWER_H
#define GDSVIEWER_H

#include <QWidget>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QComboBox>
#include <QProgressDialog>
#include "LayoutFileReader.h"

class GdsViewer : public QWidget {
    Q_OBJECT
public:
    GdsViewer(QWidget *parent = nullptr);

private slots:
    void loadGdsFile();
    void updateLayer(int index);

private:
    void renderLayer(const Layer &layer);

    QGraphicsView *graphicsView;
    QGraphicsScene *scene;
    QPushButton *loadButton;
    QComboBox *layerCombo;
    LayoutFileReader *reader;
    std::vector<std::pair<int, int>> availableLayers;
    QProgressDialog *progressDialog;
};

#endif
