#include "DatabaseManager.h"
#include <sstream>
#include <iomanip>
#include <cstdlib>

DatabaseManager::DatabaseManager(const std::string& db_name, const std::string& user,
                                 const std::string& password, const std::string& host,
                                 const std::string& port, ErrorCallback error_callback)
    : db_name_(db_name), user_(user), password_(password), host_(host), port_(port),
      error_callback_(std::move(error_callback)) {
    LOG_FUNCTION();
    if (user_.empty()) user_ = std::getenv("PGUSER") ? std::getenv("PGUSER") : "";
    if (password_.empty()) password_ = std::getenv("PGPASSWORD") ? std::getenv("PGPASSWORD") : "";
}

DatabaseManager::~DatabaseManager() {
    LOG_FUNCTION();
    disconnect();
}

void DatabaseManager::setErrorCallback(ErrorCallback callback) {
    error_callback_ = std::move(callback);
}

void DatabaseManager::reportError(const std::string& message) {
    LOG_ERROR(message);
    if (error_callback_) error_callback_(message);
}

bool DatabaseManager::isConnected() const {
    return conn_ && conn_->is_open();
}

bool DatabaseManager::connect() {
    LOG_FUNCTION();
    if (isConnected()) return true;

    try {
        std::string conn_str = "dbname=" + db_name_ + " user=" + user_ +
                               " password=" + password_ + " host=" + host_ +
                               " port=" + port_;
        conn_ = std::make_unique<pqxx::connection>(conn_str);
        LOG_INFO("Connected to database: " + db_name_);
        return true;
    } catch (const std::exception& e) {
        reportError("Connection failed: " + std::string(e.what()));
        conn_.reset();
        return false;
    }
}

void DatabaseManager::disconnect() {
    LOG_FUNCTION();
    if (isConnected()) {
        conn_.reset();
        LOG_INFO("Disconnected from database: " + db_name_);
    }
}

bool DatabaseManager::createDatabaseIfNotExists() {
    LOG_FUNCTION();
    try {
        std::string conn_str = "dbname=postgres user=" + user_ +
                               " password=" + password_ + " host=" + host_ +
                               " port=" + port_;
        pqxx::connection temp_conn(conn_str);
        pqxx::work txn(temp_conn);
        pqxx::result res = txn.exec("SELECT 1 FROM pg_database WHERE datname = '" + db_name_ + "'");
        if (res.empty()) {
            txn.exec0("CREATE DATABASE \"" + db_name_ + "\"");
            LOG_INFO("Created database: " + db_name_);
            txn.commit();
        }
        return true;
    } catch (const std::exception& e) {
        reportError("Failed to create database " + db_name_ + ": " + std::string(e.what()));
        return false;
    }
}

bool DatabaseManager::createTables() {
    LOG_FUNCTION();
    if (!isConnected() && !connect()) return false;

    try {
        pqxx::work txn(*conn_);
        txn.exec0(R"(
            CREATE TABLE IF NOT EXISTS patterns (
                id SERIAL PRIMARY KEY,
                pattern_hash VARCHAR(64) UNIQUE,
                mask_layer_number INTEGER,
                mask_layer_datatype INTEGER,
                input_layers JSONB,
                layout_file_name VARCHAR(255),
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        )");
        txn.exec0(R"(
            CREATE TABLE IF NOT EXISTS pattern_geometries (
                id SERIAL PRIMARY KEY,
                pattern_id INTEGER REFERENCES patterns(id),
                layer_number INTEGER,
                datatype INTEGER,
                geometry_type VARCHAR(20),
                coordinates JSONB,
                area DOUBLE PRECISION,
                perimeter DOUBLE PRECISION
            )
        )");
        txn.commit();
        LOG_INFO("Created tables in database: " + db_name_);
        return true;
    } catch (const std::exception& e) {
        reportError("Failed to create tables: " + std::string(e.what()));
        return false;
    }
}

bool DatabaseManager::storePattern(const MultiLayerPattern& pattern, const std::string& layout_file_name) {
    LOG_FUNCTION();
    if (!isConnected() && !connect()) return false;

    pqxx::work txn(*conn_);
    try {
        int pattern_id = insertPatternMetadata(pattern, layout_file_name);
        if (pattern_id < 0) throw std::runtime_error("Failed to insert pattern metadata");
        if (!insertPatternGeometries(pattern_id, pattern))
            throw std::runtime_error("Failed to insert pattern geometries");
        txn.commit();
        LOG_INFO("Stored pattern: " + pattern.pattern_id);
        return true;
    } catch (const std::exception& e) {
        reportError("Error storing pattern: " + std::string(e.what()));
        txn.abort();
        return false;
    }
}

int DatabaseManager::insertPatternMetadata(const MultiLayerPattern& pattern, const std::string& layout_file_name) {
    LOG_FUNCTION();
    try {
        std::ostringstream layers_stream;
        layers_stream << "[";
        for (size_t i = 0; i < pattern.input_layers.size(); ++i) {
            if (i > 0) layers_stream << ",";
            layers_stream << "{\"layer\":" << pattern.input_layers[i].layer_number
                          << ",\"datatype\":" << pattern.input_layers[i].datatype << "}";
        }
        layers_stream << "]";
        std::string layers_str = layers_stream.str();

        pqxx::work txn(*conn_);
        pqxx::result res = txn.exec_params(
            "INSERT INTO patterns (pattern_hash, mask_layer_number, mask_layer_datatype, input_layers, layout_file_name) "
            "VALUES ($1, $2, $3, $4::jsonb, $5) RETURNING id",
            pattern.pattern_id, pattern.mask_layer_number, pattern.mask_layer_datatype,
            layers_str, layout_file_name);
        if (res.empty()) throw std::runtime_error("No ID returned");
        int pattern_id = res[0][0].as<int>();
        txn.commit();
        LOG_INFO("Inserted pattern with ID: " + std::to_string(pattern_id));
        return pattern_id;
    } catch (const std::exception& e) {
        reportError("Error inserting pattern metadata: " + std::string(e.what()));
        return -1;
    }
}

bool DatabaseManager::insertPatternGeometries(int pattern_id, const MultiLayerPattern& pattern) {
    LOG_FUNCTION();
    for (const auto& layer : pattern.input_layers) {
        for (const auto& polygon : layer.polygons) {
            if (!insertPolygon(pattern_id, layer.layer_number, layer.datatype, polygon)) return false;
        }
    }
    return insertPolygon(pattern_id, pattern.mask_layer_number, pattern.mask_layer_datatype, pattern.mask_polygon);
}

bool DatabaseManager::insertPolygon(int pattern_id, int layer_number, int datatype, const Polygon& polygon) {
    LOG_FUNCTION();
    try {
        std::ostringstream json_stream;
        json_stream << std::fixed << std::setprecision(2) << "[";
        for (size_t i = 0; i < polygon.points.size(); ++i) {
            if (i > 0) json_stream << ",";
            json_stream << "[" << polygon.points[i].x << "," << polygon.points[i].y << "]";
        }
        json_stream << "]";
        std::string json_str = json_stream.str();

        pqxx::work txn(*conn_);
        txn.exec_params(
            "INSERT INTO pattern_geometries "
            "(pattern_id, layer_number, datatype, geometry_type, coordinates, area, perimeter) "
            "VALUES ($1, $2, $3, $4, $5::jsonb, $6, $7)",
            pattern_id, layer_number, datatype, "polygon", json_str, polygon.area, polygon.perimeter);
        txn.commit();
        LOG_DEBUG("Inserted polygon for layer " + std::to_string(layer_number) + ":" + std::to_string(datatype));
        return true;
    } catch (const std::exception& e) {
        reportError("Error inserting polygon: " + std::string(e.what()));
        return false;
    }
}

std::vector<Pattern> DatabaseManager::getPatterns() {
    LOG_FUNCTION();
    std::vector<Pattern> patterns;
    if (!isConnected() && !connect()) return patterns;

    try {
        pqxx::work txn(*conn_);
        pqxx::result res = txn.exec("SELECT id, pattern_hash, mask_layer_number, mask_layer_datatype, "
                                    "input_layers::text, layout_file_name, created_at "
                                    "FROM patterns ORDER BY created_at DESC");
        for (const auto& row : res) {
            patterns.push_back({
                row[0].as<int>(),
                row[1].as<std::string>(),
                row[2].as<int>(),
                row[3].as<int>(),
                row[4].as<std::string>(),
                row[5].as<std::string>(),
                row[6].as<std::string>()
            });
        }
        LOG_INFO("Retrieved " + std::to_string(patterns.size()) + " patterns");
        return patterns;
    } catch (const std::exception& e) {
        reportError("Error retrieving patterns: " + std::string(e.what()));
        return patterns;
    }
}

std::vector<Geometry> DatabaseManager::getGeometries(int pattern_id) {
    LOG_FUNCTION();
    std::vector<Geometry> geometries;
    if (!isConnected() && !connect()) return geometries;

    try {
        pqxx::work txn(*conn_);
        std::string query = "SELECT id, pattern_id, layer_number, datatype, geometry_type, "
                            "coordinates::text, area, perimeter FROM pattern_geometries";
        if (pattern_id >= 0) query += " WHERE pattern_id = " + std::to_string(pattern_id);
        pqxx::result res = txn.exec(query);
        for (const auto& row : res) {
            geometries.push_back({
                row[0].as<int>(),
                row[1].as<int>(),
                row[2].as<int>(),
                row[3].as<int>(),
                row[4].as<std::string>(),
                row[5].as<std::string>(),
                row[6].as<double>(),
                row[7].as<double>()
            });
        }
        LOG_INFO("Retrieved " + std::to_string(geometries.size()) + " geometries");
        return geometries;
    } catch (const std::exception& e) {
        reportError("Error retrieving geometries: " + std::string(e.what()));
        return geometries;
    }
}
