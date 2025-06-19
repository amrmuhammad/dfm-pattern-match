#ifndef DATABASEVIEWER_H
#define DATABASEVIEWER_H

#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QSplitter>
#include <QTabWidget>
#include <QTableView>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSettings>
#include <QSqlQueryModel>
#include "../shared/DatabaseManager.h"

class DatabaseViewer : public QWidget {
    Q_OBJECT
public:
    explicit DatabaseViewer(QWidget *parent = nullptr);
    virtual ~DatabaseViewer();

private slots:
    void configureDatabase();
    void onDatabaseSelected(QListWidgetItem *item);
    void onPatternSelected(QListWidgetItem *item);
    void refreshPatterns();

private:
    void loadDatabases();
    void loadPatterns();
    void renderPolygon(const QString &coordinates);
    void loadSettings();
    void saveSettings();

    DatabaseManager *dbManager;
    QListWidget *databasesList;
    QListWidget *patternsList;
    QLabel *databasesListLabel;
    QLabel *patternsListLabel;
    QPushButton *refreshButton;
    QPushButton *dbConfigButton;
    QTabWidget *rightTabs;
    QTableView *tableView;
    QGraphicsView *graphicsView;
    QGraphicsScene *scene;
    QSettings *settings;
};

#endif // DATABASEVIEWER_H
