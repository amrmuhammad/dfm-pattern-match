#!/bin/bash

# Regression test for DFM Pattern Capture Application
# Cleans database, runs generate_test_gds, runs dfm_pattern_capture,
# verifies logs, and checks database entries.

set -e # Exit on error

# Paths and variables
PROJECT_DIR="/home/amrmuhammad/dev/dfm_pattern_match4/dfm-pattern-capture-enhanced2"
BUILD_DIR="${PROJECT_DIR}/build"
TEST_DIR="${PROJECT_DIR}/test"
DB_NAME="test_db"
LOG_FILE="${BUILD_DIR}/test_output.log"
EXPECTED_LOG_FILE="${TEST_DIR}/expected_log.txt"
DB_OUTPUT_FILE="${BUILD_DIR}/db_output.txt"
EXPECTED_DB_FILE="${TEST_DIR}/expected_db.txt"

# Step 1: Clean up the database
echo "Cleaning up database: ${DB_NAME}"
psql -d "${DB_NAME}" -c "TRUNCATE TABLE pattern_geometries, patterns RESTART IDENTITY;" || {
    echo "Error: Failed to clean database"
    exit 1
}

# Step 2: Run generate_test_gds
echo "Running generate_test_gds"
cd "${BUILD_DIR}"
./generate_test_gds || {
    echo "Error: generate_test_gds failed"
    exit 1
}

# Step 3: Run dfm_pattern_capture and capture output
echo "Running dfm_pattern_capture"
./dfm_pattern_capture -layout_file test.gds -mask_layer_number 1 -input_layers_numbers 2 -db_name "${DB_NAME}" > "${LOG_FILE}" 2>&1 || {
    echo "Error: dfm_pattern_capture failed"
    cat "${LOG_FILE}"
    exit 1
}

# Step 4: Verify logs
echo "Verifying logs"

# Create expected log patterns
cat > "${EXPECTED_LOG_FILE}" << 'EOF'
Perimeter segment 0: dx=100, dy=0, length=100, total=100
Perimeter segment 1: dx=0, dy=100, length=100, total=200
Perimeter segment 2: dx=-100, dy=0, length=100, total=300
Perimeter segment 3: dx=0, dy=-100, length=100, total=400
Added polygon to layer 1, area=10000
Perimeter segment 0: dx=100, dy=0, length=100, total=100
Perimeter segment 1: dx=0, dy=100, length=100, total=200
Perimeter segment 2: dx=-100, dy=0, length=100, total=300
Perimeter segment 3: dx=0, dy=-100, length=100, total=400
Added polygon to layer 2, area=10000
Perimeter segment 0: dx=50, dy=0, length=50, total=50
Perimeter segment 1: dx=0, dy=50, length=50, total=100
Perimeter segment 2: dx=-50, dy=0, length=50, total=150
Perimeter segment 3: dx=0, dy=-50, length=50, total=200
Inserting polygon with coordinates: [[50.00,50.00],[100.00,50.00],[100.00,100.00],[50.00,100.00]]
Successfully stored pattern: pattern_1_10000.00_2_
Total patterns processed: 1
Successfully stored: 1
Failed: 0
EOF

# Extract and normalize relevant lines from log file
grep -E "Perimeter segment|Added polygon|Inserting polygon|Successfully stored pattern|Total patterns processed|Successfully stored|Failed" "${LOG_FILE}" | \
sed -E 's/Successfully stored pattern: pattern_1_10000\.00_2_[0-9]+/Successfully stored pattern: pattern_1_10000.00_2_/' > "${LOG_FILE}.filtered"

# Compare logs
if ! diff -u "${EXPECTED_LOG_FILE}" "${LOG_FILE}.filtered" > "${LOG_FILE}.diff"; then
    echo "Error: Log output does not match expected"
    cat "${LOG_FILE}.diff"
    exit 1
else
    echo "Logs verified successfully"
fi

# Step 5: Check database entries
echo "Checking database entries"

# Query database
psql -d "${DB_NAME}" -c "SELECT id, pattern_hash, mask_layer_number, input_layers_numbers, layout_file_name FROM patterns;" -t -A > "${DB_OUTPUT_FILE}"
psql -d "${DB_NAME}" -c "SELECT id, pattern_id, layer_number, geometry_type, coordinates::text, area, perimeter FROM pattern_geometries;" -t -A >> "${DB_OUTPUT_FILE}"

# Create expected database output
cat > "${EXPECTED_DB_FILE}" << 'EOF'
1|pattern_1_10000.00_2_|1|{2}|layout.gds
1|1|2|polygon|[[50.00,50.00],[100.00,50.00],[100.00,100.00],[50.00,100.00]]|2500|200
EOF

# Filter out timestamp from pattern_hash and normalize spaces in coordinates
sed -E 's/\|pattern_1_10000\.00_2_[0-9]+/\|pattern_1_10000.00_2_/; s/,\s+/,/g' "${DB_OUTPUT_FILE}" > "${DB_OUTPUT_FILE}.filtered"

# Compare database output
if ! diff -u "${EXPECTED_DB_FILE}" "${DB_OUTPUT_FILE}.filtered" > "${DB_OUTPUT_FILE}.diff"; then
    echo "Error: Database entries do not match expected"
    cat "${DB_OUTPUT_FILE}.diff"
    exit 1
else
    echo "Database entries verified successfully"
fi

# Clean up temporary files
rm -f "${LOG_FILE}" "${LOG_FILE}.filtered" "${LOG_FILE}.diff" "${DB_OUTPUT_FILE}" "${DB_OUTPUT_FILE}.filtered" "${DB_OUTPUT_FILE}.diff"

echo "Regression test passed successfully!"
exit 0
