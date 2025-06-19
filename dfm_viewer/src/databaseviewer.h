#ifndef DATABASEVIEWER_H
#define DATABASEVIEWER_H

#include <QWidget>
#include <QListWidget>
#include <QTableView>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPushButton>
#include <QTabWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "../shared/DatabaseManager.h"
#include <QSettings>

class DatabaseViewer : public QWidget {
    Q_OBJECT
public:
    DatabaseViewer(QWidget *parent = nullptr);

private slots:
    void refreshPatterns();
    void configureDatabase();
    void onDatabaseSelected(QListWidgetItem *item);
    void onPatternSelected(QListWidgetItem *item);

private:
    void loadPatterns();
    void renderPolygon(const QString &coordinates);
    void loadSettings();
    void saveSettings();

    QListWidget *databasesList;
    QListWidget *patternsList;
    QLabel *databasesListLabel;
    QLabel *patternsListLabel;
    QGraphicsView *graphicsView;
    QGraphicsScene *scene;
    QTableView *tableView;
    QPushButton *refreshButton;
    QPushButton *dbConfigButton;
    QTabWidget *rightTabs;
    DatabaseManager *dbManager;
    QSettings *settings;
};

#endif
