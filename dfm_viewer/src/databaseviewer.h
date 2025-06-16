#ifndef DATABASEVIEWER_H
#define DATABASEVIEWER_H

#include <QWidget>
#include <QTableView>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QtWidgets>
#include <pqxx/pqxx>

class DatabaseViewer : public QWidget {
    Q_OBJECT
public:
    DatabaseViewer(QWidget *parent = nullptr);
    
private:
    void loadPatterns();
    void renderPolygon(const QString &coordinates);
    void refreshPatterns();

    QTableView *tableView;
    QGraphicsView *graphicsView;
    QPushButton *refreshButton;
    QGraphicsScene *scene;
    pqxx::connection *conn;
};

#endif
