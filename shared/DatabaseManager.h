#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <pqxx/pqxx>
#include <string>
#include <vector>
#include <functional>
#include "Geometry.h"

struct Pattern {
    int id;
    std::string pattern_hash;
    int mask_layer_number;
    int mask_layer_datatype;
    std::string input_layers;
    std::string layout_file_name;
    std::string created_at;
};

struct Geometry {
    int id;
    int pattern_id;
    int layer_number;
    int datatype;
    std::string geometry_type;
    std::string coordinates;
    double area;
    double perimeter;
};

using ErrorCallback = std::function<void(const std::string&)>;

class DatabaseManager {
public:
    DatabaseManager(const std::string& db_name, const std::string& user,
                    const std::string& password, const std::string& host,
                    const std::string& port, ErrorCallback error_callback);
    ~DatabaseManager();
    void setErrorCallback(ErrorCallback callback);
    bool isConnected() const;
    bool connect();
    void disconnect();
    bool createDatabaseIfNotExists();
    bool createTables();
    bool isValidSchema();
    bool storePattern(const MultiLayerPattern& pattern, const std::string& layout_file_name);
    std::vector<Pattern> getPatterns();
    std::vector<Geometry> getGeometries(int pattern_id = -1);
    std::vector<std::string> getAvailableDatabases();

private:
    std::string db_name_;
    std::string user_;
    std::string password_;
    std::string host_;
    std::string port_;
    std::unique_ptr<pqxx::connection> conn_;
    ErrorCallback error_callback_;
    void reportError(const std::string& message, const std::string& query = "");
    int insertPatternMetadata(pqxx::work& txn, const MultiLayerPattern& pattern, const std::string& layout_file_name);
    bool insertPatternGeometries(pqxx::work& txn, int pattern_id, const MultiLayerPattern& pattern);
    bool insertPolygon(pqxx::work& txn, int pattern_id, int layer_number, int datatype, const Polygon& polygon);
};

#endif // DATABASEMANAGER_H
