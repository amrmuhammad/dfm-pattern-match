#ifndef DATABASEVIEWER_H
#define DATABASEVIEWER_H

#include <QWidget>
#include <QSettings>
#include <QTableView>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPushButton>
#include <QVBoxLayout>
#include "../shared/DatabaseManager.h"

class DatabaseViewer : public QWidget {
    Q_OBJECT
public:
    explicit DatabaseViewer(QWidget *parent = nullptr);
private slots:
    void refreshPatterns();
    void configureDatabase();
private:
    QSettings* settings;
    DatabaseManager* db_manager;
    QPushButton* refreshButton;
    QPushButton* dbConfigButton;
    QTableView* tableView;
    QGraphicsView* graphicsView;
    QGraphicsScene* scene;

    void loadPatterns();
    void renderPolygon(const QString &coordinates);
    void loadSettings();
    void saveSettings();
};

#endif // DATABASEVIEWER_H
