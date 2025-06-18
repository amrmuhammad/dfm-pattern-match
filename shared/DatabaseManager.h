#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include "Geometry.h"
#include "Logging.h"
#include <pqxx/pqxx>
#include <string>
#include <vector>
#include <functional>

struct Pattern {
    int id;
    std::string pattern_hash;
    int mask_layer_number;
    int mask_layer_datatype;
    std::string input_layers; // JSON string
    std::string layout_file_name;
    std::string created_at;
};

struct Geometry {
    int id;
    int pattern_id;
    int layer_number;
    int datatype;
    std::string geometry_type;
    std::string coordinates; // JSON string
    double area;
    double perimeter;
};

class DatabaseManager {
public:
    using ErrorCallback = std::function<void(const std::string&)>;

    explicit DatabaseManager(const std::string& db_name, const std::string& user = "",
                             const std::string& password = "", const std::string& host = "localhost",
                             const std::string& port = "5432", ErrorCallback error_callback = nullptr);
    ~DatabaseManager();

    bool isConnected() const;
    bool connect();
    void disconnect();
    bool createDatabaseIfNotExists();
    bool createTables();
    bool storePattern(const MultiLayerPattern& pattern, const std::string& layout_file_name);
    std::vector<Pattern> getPatterns();
    std::vector<Geometry> getGeometries(int pattern_id = -1); // -1 for all geometries

    void setErrorCallback(ErrorCallback callback);

private:
    std::string db_name_;
    std::string user_;
    std::string password_;
    std::string host_;
    std::string port_;
    std::unique_ptr<pqxx::connection> conn_;
    ErrorCallback error_callback_;

    bool executeQuery(const std::string& query);
    int insertPatternMetadata(const MultiLayerPattern& pattern, const std::string& layout_file_name);
    bool insertPatternGeometries(int pattern_id, const MultiLayerPattern& pattern);
    bool insertPolygon(int pattern_id, int layer_number, int datatype, const Polygon& polygon);
    void reportError(const std::string& message);
};

#endif // DATABASE_MANAGER_H
