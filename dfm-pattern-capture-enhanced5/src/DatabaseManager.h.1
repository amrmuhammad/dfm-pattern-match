#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include "Geometry.h"
#include <string>
#include <libpq-fe.h>

class DatabaseManager {
public:
    DatabaseManager(const std::string& db_name);
    ~DatabaseManager();
    bool storePattern(const MultiLayerPattern& pattern, const std::string& layout_file_name);
private:
    std::string db_name_;
    PGconn* conn_;
    bool createDatabaseIfNotExists();
    void connect();
    void createTables();
    int insertPatternMetadata(const MultiLayerPattern& pattern, const std::string& layout_file_name);
    bool insertPatternGeometries(int pattern_id, const MultiLayerPattern& pattern);
    bool insertPolygon(int pattern_id, int layer_number, int datatype, const Polygon& polygon);
};

#endif
