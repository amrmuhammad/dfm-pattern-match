#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include "Geometry.h"
#include <string>
#include <libpq-fe.h>

class DatabaseManager {
public:
    explicit DatabaseManager(const std::string& db_name);
    ~DatabaseManager();
    bool storePattern(const MultiLayerPattern& pattern);

private:
    std::string db_name_;
    PGconn* conn_;
    void connect();
    bool createDatabaseIfNotExists(); // New method
    void createTables();
    int insertPatternMetadata(const MultiLayerPattern& pattern);
    bool insertPatternGeometries(int pattern_id, const MultiLayerPattern& pattern);
    bool insertPolygon(int pattern_id, int layer_number, const Polygon& polygon);
};

#endif // DATABASEMANAGER_H
