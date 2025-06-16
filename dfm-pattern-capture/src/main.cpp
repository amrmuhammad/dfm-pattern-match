#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <sstream>
#include <iomanip>        // Add this line
#include <libpq-fe.h>

// Forward declarations
struct Point;
struct Polygon;
struct Layer;
struct MultiLayerPattern;
class LayoutFileReader;
class GeometryProcessor;
class DatabaseManager;

// Data structures
struct Point {
    double x, y;
    Point(double x = 0.0, double y = 0.0) : x(x), y(y) {}
    
    bool operator==(const Point& other) const {
        return std::abs(x - other.x) < 1e-9 && std::abs(y - other.y) < 1e-9;
    }
};

struct Polygon {
    std::vector<Point> points;
    double area;
    
    Polygon() : area(0.0) {}
    
    // Calculate polygon area using shoelace formula
    void calculateArea() {
        if (points.size() < 3) {
            area = 0.0;
            return;
        }
        
        double sum = 0.0;
        for (size_t i = 0; i < points.size(); ++i) {
            size_t j = (i + 1) % points.size();
            sum += points[i].x * points[j].y - points[j].x * points[i].y;
        }
        area = std::abs(sum) / 2.0;
    }
    
    // Check if polygon is valid (at least 3 points, non-zero area)
    bool isValid() const {
        return points.size() >= 3 && area > 1e-9;
    }
};

struct Layer {
    int layer_number;
    std::vector<Polygon> polygons;
    
    Layer(int num = -1) : layer_number(num) {}
    
    size_t getPolygonCount() const { return polygons.size(); }
    double getTotalArea() const {
        double total = 0.0;
        for (const auto& poly : polygons) {
            total += poly.area;
        }
        return total;
    }
};

struct MultiLayerPattern {
    std::string pattern_id;
    int mask_layer_number;
    Polygon mask_polygon;
    std::vector<Layer> input_layers;
    std::chrono::system_clock::time_point created_at;
    
    MultiLayerPattern() : mask_layer_number(-1), created_at(std::chrono::system_clock::now()) {}
};

// Command line arguments structure
struct CommandLineArgs {
    std::string layout_file;
    int mask_layer_number = -1;
    std::vector<int> input_layers_numbers;
    std::string db_name;
    
    bool isValid() const {
        return !layout_file.empty() && 
               mask_layer_number >= 0 && 
               !input_layers_numbers.empty() && 
               !db_name.empty();
    }
};

// Layout file reader class
class LayoutFileReader {
public:
    enum FileType { GDSII, OASIS, UNKNOWN };
    
    explicit LayoutFileReader(const std::string& filename) : filename_(filename) {
        detectFileType();
    }
    
    FileType getFileType() const { return file_type_; }
    
    // Load specific layer from the layout file
    Layer loadLayer(int layer_number) {
        Layer layer(layer_number);
        
        if (file_type_ == GDSII) {
            loadGDSIILayer(layer_number, layer);
        } else if (file_type_ == OASIS) {
            loadOASISLayer(layer_number, layer);
        } else {
            throw std::runtime_error("Unsupported file format");
        }
        
        return layer;
    }
    
    // Get available layers in the file
    std::vector<int> getAvailableLayers() {
        // TODO: Implement layer discovery
        // For now, return dummy data
        return {1, 2, 3, 4, 5, 10, 20, 30};
    }

private:
    std::string filename_;
    FileType file_type_;
    
    void detectFileType() {
        std::string ext = filename_.substr(filename_.find_last_of('.'));
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        
        if (ext == ".gds" || ext == ".gdsii") {
            file_type_ = GDSII;
        } else if (ext == ".oas" || ext == ".oasis") {
            file_type_ = OASIS;
        } else {
            file_type_ = UNKNOWN;
        }
    }
    
    void loadGDSIILayer(int layer_number, Layer& layer) {
        // TODO: Implement GDSII parsing using libgds or similar
        // For now, create dummy polygons for testing
        std::cout << "Loading GDSII layer " << layer_number << " from " << filename_ << std::endl;
        
        // Create sample rectangles for testing
        for (int i = 0; i < 5; ++i) {
            Polygon poly;
            double x = i * 100.0;
            double y = i * 50.0;
            poly.points = {
                Point(x, y),
                Point(x + 50, y),
                Point(x + 50, y + 25),
                Point(x, y + 25)
            };
            poly.calculateArea();
            layer.polygons.push_back(poly);
        }
    }
    
    void loadOASISLayer(int layer_number, Layer& layer) {
        // TODO: Implement OASIS parsing
        std::cout << "Loading OASIS layer " << layer_number << " from " << filename_ << std::endl;
        
        // Create sample polygons for testing
        for (int i = 0; i < 3; ++i) {
            Polygon poly;
            double x = i * 80.0;
            double y = i * 60.0;
            poly.points = {
                Point(x, y),
                Point(x + 40, y),
                Point(x + 40, y + 30),
                Point(x, y + 30)
            };
            poly.calculateArea();
            layer.polygons.push_back(poly);
        }
    }
};

// Geometry processing class
class GeometryProcessor {
public:
    // Perform AND operation between mask polygon and input layer polygons
    static Layer performANDOperation(const Polygon& mask_polygon, const Layer& input_layer) {
        Layer result_layer(input_layer.layer_number);
        
        std::cout << "Performing AND operation on layer " << input_layer.layer_number 
                  << " with " << input_layer.polygons.size() << " polygons" << std::endl;
        
        for (const auto& input_polygon : input_layer.polygons) {
            Polygon intersection = intersectPolygons(mask_polygon, input_polygon);
            if (intersection.isValid()) {
                result_layer.polygons.push_back(intersection);
            }
        }
        
        std::cout << "AND operation resulted in " << result_layer.polygons.size() << " polygons" << std::endl;
        return result_layer;
    }

private:
    // Simple polygon intersection (placeholder implementation)
    // TODO: Implement proper polygon clipping algorithm (Sutherland-Hodgman, etc.)
    static Polygon intersectPolygons(const Polygon& poly1, const Polygon& poly2) {
        Polygon result;
        
        // Simplified intersection: check if poly2 is inside poly1's bounding box
        auto [min1_x, max1_x, min1_y, max1_y] = getBoundingBox(poly1);
        auto [min2_x, max2_x, min2_y, max2_y] = getBoundingBox(poly2);
        
        // Simple overlap check
        if (max1_x >= min2_x && max2_x >= min1_x && max1_y >= min2_y && max2_y >= min1_y) {
            // Create intersection rectangle (simplified)
            double int_min_x = std::max(min1_x, min2_x);
            double int_max_x = std::min(max1_x, max2_x);
            double int_min_y = std::max(min1_y, min2_y);
            double int_max_y = std::min(max1_y, max2_y);
            
            if (int_max_x > int_min_x && int_max_y > int_min_y) {
                result.points = {
                    Point(int_min_x, int_min_y),
                    Point(int_max_x, int_min_y),
                    Point(int_max_x, int_max_y),
                    Point(int_min_x, int_max_y)
                };
                result.calculateArea();
            }
        }
        
        return result;
    }
    
    static std::tuple<double, double, double, double> getBoundingBox(const Polygon& poly) {
        if (poly.points.empty()) {
            return {0, 0, 0, 0};
        }
        
        double min_x = poly.points[0].x, max_x = poly.points[0].x;
        double min_y = poly.points[0].y, max_y = poly.points[0].y;
        
        for (const auto& point : poly.points) {
            min_x = std::min(min_x, point.x);
            max_x = std::max(max_x, point.x);
            min_y = std::min(min_y, point.y);
            max_y = std::max(max_y, point.y);
        }
        
        return {min_x, max_x, min_y, max_y};
    }
};

// Database manager class
class DatabaseManager {
public:
    explicit DatabaseManager(const std::string& db_name) : db_name_(db_name), conn_(nullptr) {
        connect();
        createTables();
    }
    
    ~DatabaseManager() {
        if (conn_) {
            PQfinish(conn_);
        }
    }
    
    bool storePattern(const MultiLayerPattern& pattern) {
        if (!conn_) {
            std::cerr << "No database connection" << std::endl;
            return false;
        }
        
        // Start transaction
        PGresult* res = PQexec(conn_, "BEGIN");
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            std::cerr << "BEGIN command failed: " << PQerrorMessage(conn_) << std::endl;
            PQclear(res);
            return false;
        }
        PQclear(res);
        
        try {
            // Insert pattern metadata
            int pattern_id = insertPatternMetadata(pattern);
            if (pattern_id < 0) {
                throw std::runtime_error("Failed to insert pattern metadata");
            }
            
            // Insert pattern geometries
            if (!insertPatternGeometries(pattern_id, pattern)) {
                throw std::runtime_error("Failed to insert pattern geometries");
            }
            
            // Commit transaction
            res = PQexec(conn_, "COMMIT");
            if (PQresultStatus(res) != PGRES_COMMAND_OK) {
                throw std::runtime_error("COMMIT failed");
            }
            PQclear(res);
            
            std::cout << "Successfully stored pattern: " << pattern.pattern_id << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cerr << "Error storing pattern: " << e.what() << std::endl;
            
            // Rollback transaction
            res = PQexec(conn_, "ROLLBACK");
            PQclear(res);
            return false;
        }
    }

private:
    std::string db_name_;
    PGconn* conn_;
    
    void connect() {
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
    
    void createTables() {
        if (!conn_) return;
        
        const char* create_patterns_table = R"(
            CREATE TABLE IF NOT EXISTS patterns (
                id SERIAL PRIMARY KEY,
                pattern_hash VARCHAR(64) UNIQUE,
                mask_layer_number INTEGER,
                input_layers_numbers INTEGER[],
                layout_file_name VARCHAR(255),
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        )";
        
        const char* create_geometries_table = R"(
            CREATE TABLE IF NOT EXISTS pattern_geometries (
                id SERIAL PRIMARY KEY,
                pattern_id INTEGER REFERENCES patterns(id),
                layer_number INTEGER,
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
    
    int insertPatternMetadata(const MultiLayerPattern& pattern) {
        // Create input layers array string
        std::ostringstream layers_stream;
        layers_stream << "{";
        for (size_t i = 0; i < pattern.input_layers.size(); ++i) {
            if (i > 0) layers_stream << ",";
            layers_stream << pattern.input_layers[i].layer_number;
        }
        layers_stream << "}";
        
        const char* query = R"(
            INSERT INTO patterns 
            (pattern_hash, mask_layer_number, input_layers_numbers, layout_file_name) 
            VALUES ($1, $2, $3, $4) 
            RETURNING id
        )";
        
        const char* params[4] = {
            pattern.pattern_id.c_str(),
            std::to_string(pattern.mask_layer_number).c_str(),
            layers_stream.str().c_str(),
            "layout.gds" // TODO: Get actual filename
        };
        
        PGresult* res = PQexecParams(conn_, query, 4, nullptr, params, nullptr, nullptr, 0);
        
        if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) != 1) {
            std::cerr << "INSERT pattern failed: " << PQerrorMessage(conn_) << std::endl;
            PQclear(res);
            return -1;
        }
        
        int pattern_id = std::atoi(PQgetvalue(res, 0, 0));
        PQclear(res);
        return pattern_id;
    }
    
    bool insertPatternGeometries(int pattern_id, const MultiLayerPattern& pattern) {
        for (const auto& layer : pattern.input_layers) {
            for (const auto& polygon : layer.polygons) {
                if (!insertPolygon(pattern_id, layer.layer_number, polygon)) {
                    return false;
                }
            }
        }
        return true;
    }
    
    bool insertPolygon(int pattern_id, int layer_number, const Polygon& polygon) {
        // Convert polygon to JSON
        std::ostringstream json_stream;
        json_stream << "[";
        for (size_t i = 0; i < polygon.points.size(); ++i) {
            if (i > 0) json_stream << ",";
            json_stream << "[" << polygon.points[i].x << "," << polygon.points[i].y << "]";
        }
        json_stream << "]";
        
        const char* query = R"(
            INSERT INTO pattern_geometries 
            (pattern_id, layer_number, geometry_type, coordinates, area) 
            VALUES ($1, $2, $3, $4, $5)
        )";
        
        const char* params[5] = {
            std::to_string(pattern_id).c_str(),
            std::to_string(layer_number).c_str(),
            "polygon",
            json_stream.str().c_str(),
            std::to_string(polygon.area).c_str()
        };
        
        PGresult* res = PQexecParams(conn_, query, 5, nullptr, params, nullptr, nullptr, 0);
        
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            std::cerr << "INSERT geometry failed: " << PQerrorMessage(conn_) << std::endl;
            PQclear(res);
            return false;
        }
        
        PQclear(res);
        return true;
    }
};

// Utility functions
namespace Utils {
    CommandLineArgs parseCommandLine(int argc, char* argv[]) {
        CommandLineArgs args;
        
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            
            if (arg == "-layout_file" && i + 1 < argc) {
                args.layout_file = argv[++i];
            } else if (arg == "-mask_layer_number" && i + 1 < argc) {
                args.mask_layer_number = std::stoi(argv[++i]);
            } else if (arg == "-input_layers_numbers" && i + 1 < argc) {
                std::string layers_str = argv[++i];
                std::istringstream iss(layers_str);
                std::string layer;
                while (iss >> layer) {
                    args.input_layers_numbers.push_back(std::stoi(layer));
                }
            } else if (arg == "-db_name" && i + 1 < argc) {
                args.db_name = argv[++i];
            }
        }
        
        return args;
    }
    
    void printUsage(const char* program_name) {
        std::cout << "Usage: " << program_name << " [OPTIONS]\n"
                  << "Options:\n"
                  << "  -layout_file <file>           Path to OASIS or GDSII layout file\n"
                  << "  -mask_layer_number <number>   Layer number for mask layer\n"
                  << "  -input_layers_numbers <nums>  Space-separated input layer numbers\n"
                  << "  -db_name <database>           PostgreSQL database name\n"
                  << "\nExample:\n"
                  << "  " << program_name << " -layout_file design.gds -mask_layer_number 1 "
                  << "-input_layers_numbers \"2 3 4\" -db_name patterns_db\n";
    }
    
    std::string generatePatternId(int mask_layer, const Polygon& mask_poly, 
                                 const std::vector<int>& input_layers) {
        // Simple pattern ID generation based on mask polygon area and layers
        std::ostringstream id_stream;
        id_stream << "pattern_" << mask_layer << "_" 
                  << std::fixed << std::setprecision(2) << mask_poly.area << "_";
        for (int layer : input_layers) {
            id_stream << layer << "_";
        }
        
        // Add timestamp for uniqueness
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        id_stream << time_t;
        
        return id_stream.str();
    }
}

// Main application class
class DFMPatternCaptureApplication {
public:
    int run(int argc, char* argv[]) {
        try {
            // Parse command line arguments
            auto args = Utils::parseCommandLine(argc, argv);
            
            if (!args.isValid()) {
                Utils::printUsage(argv[0]);
                return 1;
            }
            
            printWelcomeMessage(args);
            
            // Initialize components
            LayoutFileReader reader(args.layout_file);
            DatabaseManager db_manager(args.db_name);
            
            // Load mask layer
            Layer mask_layer = reader.loadLayer(args.mask_layer_number);
            std::cout << "Loaded mask layer " << args.mask_layer_number 
                      << " with " << mask_layer.getPolygonCount() << " polygons" << std::endl;
            
            if (mask_layer.getPolygonCount() == 0) {
                std::cerr << "No polygons found in mask layer " << args.mask_layer_number << std::endl;
                return 1;
            }
            
            // Process patterns
            return processPatterns(reader, db_manager, mask_layer, args);
            
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
    }

private:
    void printWelcomeMessage(const CommandLineArgs& args) {
        std::cout << "========================================\n"
                  << "DFM Pattern Capture Application\n"
                  << "========================================\n"
                  << "Layout file: " << args.layout_file << "\n"
                  << "Mask layer: " << args.mask_layer_number << "\n"
                  << "Input layers: ";
        for (size_t i = 0; i < args.input_layers_numbers.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << args.input_layers_numbers[i];
        }
        std::cout << "\nDatabase: " << args.db_name << "\n"
                  << "========================================\n";
    }
    
    int processPatterns(LayoutFileReader& reader, DatabaseManager& db_manager,
                       const Layer& mask_layer, const CommandLineArgs& args) {
        int pattern_count = 0;
        int success_count = 0;
        
        // Process each polygon in mask layer
        for (const auto& mask_polygon : mask_layer.polygons) {
            std::cout << "\nProcessing mask polygon " << (pattern_count + 1) 
                      << " (area: " << mask_polygon.area << ")" << std::endl;
            
            // Create multi-layer pattern
            MultiLayerPattern pattern;
            pattern.pattern_id = Utils::generatePatternId(args.mask_layer_number, 
                                                         mask_polygon, args.input_layers_numbers);
            pattern.mask_layer_number = args.mask_layer_number;
            pattern.mask_polygon = mask_polygon;
            
            // Process each input layer
            for (int input_layer_num : args.input_layers_numbers) {
                Layer input_layer = reader.loadLayer(input_layer_num);
                std::cout << "Processing input layer " << input_layer_num 
                          << " with " << input_layer.getPolygonCount() << " polygons" << std::endl;
                
                // Perform AND operation
                Layer result_layer = GeometryProcessor::performANDOperation(mask_polygon, input_layer);
                pattern.input_layers.push_back(result_layer);
            }
            
            // Store pattern in database
            if (db_manager.storePattern(pattern)) {
                success_count++;
            } else {
                std::cerr << "Failed to store pattern: " << pattern.pattern_id << std::endl;
            }
            
            pattern_count++;
        }
        
        // Print summary
        std::cout << "\n========================================\n"
                  << "Processing Summary\n"
                  << "========================================\n"
                  << "Total patterns processed: " << pattern_count << "\n"
                  << "Successfully stored: " << success_count << "\n"
                  << "Failed: " << (pattern_count - success_count) << "\n"
                  << "========================================\n";
        
        return (success_count == pattern_count) ? 0 : 1;
    }
};

// Main function
int main(int argc, char* argv[]) {
    DFMPatternCaptureApplication app;
    return app.run(argc, argv);
}
