#include "DatabaseManager.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <Logging.h>

DatabaseManager::DatabaseManager(const std::string& db_name) : db_name_(db_name), conn_(nullptr) {
    LOG_FUNCTION()
    
    if (createDatabaseIfNotExists()) {
        connect();
        createTables();
    } else {
        std::cerr << "Failed to ensure database exists: " << db_name_ << std::endl;
    }
}

DatabaseManager::~DatabaseManager() {
    if (conn_) PQfinish(conn_);
}

bool DatabaseManager::createDatabaseIfNotExists() {
    LOG_FUNCTION()
    
    std::string conn_string = "dbname=postgres";
    PGconn* temp_conn = PQconnectdb(conn_string.c_str());
    if (PQstatus(temp_conn) != CONNECTION_OK) {
        std::cerr << "Connection to postgres database failed: " << PQerrorMessage(temp_conn) << std::endl;
        PQfinish(temp_conn);
        return false;
    }

    std::string query = "SELECT 1 FROM pg_database WHERE datname = '" + db_name_ + "'";
    PGresult* res = PQexec(temp_conn, query.c_str());
    bool db_exists = (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0);
    PQclear(res);

    if (!db_exists) {
        query = "CREATE DATABASE \"" + db_name_ + "\"";
        res = PQexec(temp_conn, query.c_str());
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            std::cerr << "Failed to create database " << db_name_ << ": " << PQerrorMessage(temp_conn) << std::endl;
            PQclear(res);
            PQfinish(temp_conn);
            return false;
        }
        std::cout << "Created database: " << db_name_ << std::endl;
        PQclear(res);
    }

    PQfinish(temp_conn);
    return true;
}

void DatabaseManager::connect() {
    LOG_FUNCTION()
    
    std::string conn_string = "dbname=" + db_name_;
    conn_ = PQconnectdb(conn_string.c_str());
    if (PQstatus(conn_) != CONNECTION_OK) {
        std::cerr << "Connection to database failed: " << PQerrorMessage(conn_) << std::endl;
        PQfinish(conn_);
        conn_ = nullptr;
    } else {
        std::cout << "Connected to database: " << db_name_ << std::endl;
    }
}

void DatabaseManager::createTables() {
    LOG_FUNCTION()
    
    if (!conn_) return;
    const char* create_patterns_table = R"(
        CREATE TABLE IF NOT EXISTS patterns (
            id SERIAL PRIMARY KEY,
            pattern_hash VARCHAR(64) UNIQUE,
            mask_layer_number INTEGER,
            mask_layer_datatype INTEGER,
            input_layers JSONB,
            layout_file_name VARCHAR(255),
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    )";
    const char* create_geometries_table = R"(
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
    )";
    PGresult* res = PQexec(conn_, create_patterns_table);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "CREATE TABLE patterns failed: " << PQerrorMessage(conn_) << std::endl;
    }
    PQclear(res);
    res = PQexec(conn_, create_geometries_table);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "CREATE TABLE pattern_geometries failed: " << PQerrorMessage(conn_) << std::endl;
    }
    PQclear(res);
}

bool DatabaseManager::storePattern(const MultiLayerPattern& pattern, const std::string& layout_file_name) {
    LOG_FUNCTION()
    
    if (!conn_) {
        std::cerr << "No database connection" << std::endl;
        return false;
    }
    PGresult* res = PQexec(conn_, "BEGIN");
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "BEGIN command failed: " << PQerrorMessage(conn_) << std::endl;
        PQclear(res);
        return false;
    }
    PQclear(res);
    try {
        int pattern_id = insertPatternMetadata(pattern, layout_file_name);
        if (pattern_id < 0) throw std::runtime_error("Failed to insert pattern metadata");
        if (!insertPatternGeometries(pattern_id, pattern))
            throw std::runtime_error("Failed to insert pattern geometries");
        res = PQexec(conn_, "COMMIT");
        if (PQresultStatus(res) != PGRES_COMMAND_OK) throw std::runtime_error("COMMIT failed: " + std::string(PQerrorMessage(conn_)));
        PQclear(res);
        std::cout << "Successfully stored pattern: " << pattern.pattern_id << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error storing pattern: " << e.what() << std::endl;
        res = PQexec(conn_, "ROLLBACK");
        PQclear(res);
        return false;
    }
}

int DatabaseManager::insertPatternMetadata(const MultiLayerPattern& pattern, const std::string& layout_file_name) {
    LOG_FUNCTION()
    
    try {
        // Debug input_layers content
        std::cout << "Input layers size: " << pattern.input_layers.size() << std::endl;
        for (size_t i = 0; i < pattern.input_layers.size(); ++i) {
            std::cout << "Input layer " << i << ": layer_number=" << pattern.input_layers[i].layer_number
                      << ", datatype=" << pattern.input_layers[i].datatype
                      << ", polygons=" << pattern.input_layers[i].polygons.size() << std::endl;
            // Validate layer_number and datatype
            if (pattern.input_layers[i].layer_number < 0 || pattern.input_layers[i].datatype < 0) {
                std::cerr << "Invalid layer data at index " << i << std::endl;
                return -1;
            }
        }

        // Manual JSON construction with validation
        std::ostringstream layers_stream;
        layers_stream << "[";
        for (size_t i = 0; i < pattern.input_layers.size(); ++i) {
            if (i > 0) layers_stream << ",";
            layers_stream << "{\"layer\":" << pattern.input_layers[i].layer_number
                          << ",\"datatype\":" << pattern.input_layers[i].datatype << "}";
        }
        layers_stream << "]";
        std::string layers_str = layers_stream.str();

        // Validate JSON string for UTF-8 compliance
        std::cout << "Serialized input_layers JSON: " << layers_str << std::endl;
        for (size_t i = 0; i < layers_str.size(); ++i) {
            if (static_cast<unsigned char>(layers_str[i]) > 0x7F) {
                std::cerr << "Error: Non-ASCII character 0x" << std::hex
                          << static_cast<int>(static_cast<unsigned char>(layers_str[i]))
                          << " in JSON at position " << std::dec << i << std::endl;
                return -1;
            }
        }

        const char* params[5] = {
            pattern.pattern_id.c_str(),
            std::to_string(pattern.mask_layer_number).c_str(),
            std::to_string(pattern.mask_layer_datatype).c_str(),
            layers_str.c_str(),
            layout_file_name.c_str()
        };
        std::cout << "Executing query: "
                  << "INSERT INTO patterns (pattern_hash, mask_layer_number, mask_layer_datatype, input_layers, layout_file_name) "
                  << "VALUES ($1, $2, $3, $4::jsonb, $5) RETURNING id with params: "
                  << "pattern_hash=" << params[0] << ", "
                  << "mask_layer=" << params[1] << ":" << params[2] << ", "
                  << "input_layers=" << params[3] << ", "
                  << "layout_file=" << params[4] << std::endl;

        PGresult* res = PQexecParams(conn_, 
            "INSERT INTO patterns (pattern_hash, mask_layer_number, mask_layer_datatype, input_layers, layout_file_name) "
            "VALUES ($1, $2, $3, $4::jsonb, $5) RETURNING id",
            5, nullptr, params, nullptr, nullptr, 0);
        
        if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) != 1) {
            std::cerr << "INSERT pattern failed: " << PQerrorMessage(conn_) << std::endl;
            PQclear(res);
            return -1;
        }
        int pattern_id = std::atoi(PQgetvalue(res, 0, 0));
        PQclear(res);
        std::cout << "Inserted pattern with ID: " << pattern_id << std::endl;
        return pattern_id;
    } catch (const std::exception& e) {
        std::cerr << "Error inserting pattern metadata: " << e.what() << std::endl;
        return -1;
    }
}

bool DatabaseManager::insertPatternGeometries(int pattern_id, const MultiLayerPattern& pattern) {
    LOG_FUNCTION()
    
    for (const auto& layer : pattern.input_layers) {
        for (const auto& polygon : layer.polygons) {
            if (!insertPolygon(pattern_id, layer.layer_number, layer.datatype, polygon)) return false;
        }
    }
    return insertPolygon(pattern_id, pattern.mask_layer_number, pattern.mask_layer_datatype, pattern.mask_polygon);
}

bool DatabaseManager::insertPolygon(int pattern_id, int layer_number, int datatype, const Polygon& polygon) {
    LOG_FUNCTION()
    
    std::ostringstream json_stream;
    json_stream << std::fixed << std::setprecision(2) << "[";
    for (size_t i = 0; i < polygon.points.size(); ++i) {
        if (i > 0) json_stream << ",";
        json_stream << "[" << polygon.points[i].x << "," << polygon.points[i].y << "]";
    }
    json_stream << "]";
    std::string json_str = json_stream.str();

    // Validate JSON string
    std::cout << "Inserting polygon for layer " << layer_number << ":" << datatype
              << " with coordinates: " << json_str << std::endl;
    for (size_t i = 0; i < json_str.size(); ++i) {
        if (static_cast<unsigned char>(json_str[i]) > 0x7F) {
            std::cerr << "Error: Non-ASCII character 0x" << std::hex
                      << static_cast<int>(static_cast<unsigned char>(json_str[i]))
                      << " in JSON at position " << std::dec << i << std::endl;
            return false;
        }
    }

    const char* query = R"(
        INSERT INTO pattern_geometries 
        (pattern_id, layer_number, datatype, geometry_type, coordinates, area, perimeter) 
        VALUES ($1, $2, $3, $4, $5::jsonb, $6, $7)
    )";
    const char* params[7] = {
        std::to_string(pattern_id).c_str(),
        std::to_string(layer_number).c_str(),
        std::to_string(datatype).c_str(),
        "polygon",
        json_str.c_str(),
        std::to_string(polygon.area).c_str(),
        std::to_string(polygon.perimeter).c_str()
    };
    PGresult* res = PQexecParams(conn_, query, 7, nullptr, params, nullptr, nullptr, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "INSERT geometry failed: " << PQerrorMessage(conn_) << std::endl;
        PQclear(res);
        return false;
    }
    PQclear(res);
    return true;
}
