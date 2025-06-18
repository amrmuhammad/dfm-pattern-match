#!/bin/bash

# Test script for DFM Pattern Capture with spm.gds
set -e

# Paths and variables
PROJECT_DIR="/home/amrmuhammad/dev/dfm_pattern_match4/dfm-pattern-capture-enhanced5"
BUILD_DIR="${PROJECT_DIR}/build"
TEST_DIR="${PROJECT_DIR}/test/test_spm"
DB_NAME="test_db"
LOG_FILE="${TEST_DIR}/spm_test_output.log"
EXPECTED_LOG_FILE="${TEST_DIR}/spm_expected_log.txt"
DB_OUTPUT_FILE="${TEST_DIR}/spm_db_output.txt"
EXPECTED_DB_FILE="${TEST_DIR}/spm_expected_db.txt"

export DFM_LOGGING=1
export DFM_LOG_LEVEL=INFO

# Step 1: Drop and recreate the database
echo "Dropping and recreating database: ${DB_NAME}"
psql -d postgres -c "DROP DATABASE IF EXISTS ${DB_NAME};" || {
    echo "Error: Failed to drop database"
    exit 1
}
psql -d postgres -c "CREATE DATABASE ${DB_NAME};" || {
    echo "Error: Failed to create database"
    exit 1
}

# Step 2: Run dfm_pattern_capture
echo "Running dfm_pattern_capture with spm.gds"
cd "${TEST_DIR}"
COMMAND="${BUILD_DIR}/dfm_pattern_capture --layout_file ./spm.gds --mask_layer_number 66 --mask_layer_datatype 20 --input_layers \"67:20,68:20,69:20\" --db_name ${DB_NAME}"
echo "Executing: ${COMMAND}"
eval ${COMMAND} > "${LOG_FILE}" 2>&1 || {
    echo "Error: dfm_pattern_capture failed with exit code $?"
    cat "${LOG_FILE}"
    exit 1
}


# Step 4: Verify logs
echo "Verifying logs"

# Create expected log patterns
cat > "${EXPECTED_LOG_FILE}" << 'EOF'
Added layer 67:20 to pattern with 2 polygons
Added layer 68:20 to pattern with 3 polygons
Added layer 69:20 to pattern with 2 polygons
Pattern input_layers size: 3
Serialized input_layers JSON: [{"layer":67,"datatype":20},{"layer":68,"datatype":20},{"layer":69,"datatype":20}]
Inserting polygon for layer 67:20 with coordinates: [[1.20,1.20],[1.80,1.20],[1.80,1.80],[1.20,1.80]]
Inserting polygon for layer 67:20 with coordinates: [[1.80,1.80],[2.40,1.80],[2.40,2.40],[1.80,2.40]]
Inserting polygon for layer 68:20 with coordinates: [[1.20,1.20],[1.60,1.20],[1.40,1.60]]
Inserting polygon for layer 68:20 with coordinates: [[1.64,1.24],[2.04,1.24],[1.84,1.64]]
Inserting polygon for layer 68:20 with coordinates: [[1.28,1.68],[1.68,1.68],[1.48,2.08]]
Inserting polygon for layer 69:20 with coordinates: [[1.40,1.40],[1.80,1.40],[1.80,1.80],[1.40,1.80]]
Inserting polygon for layer 69:20 with coordinates: [[2.00,1.40],[2.40,1.40],[2.40,1.80],[2.00,1.80]]
Inserting polygon for layer 66:20 with coordinates: [[1.00,1.00],[3.00,1.00],[3.00,3.00],[1.00,3.00]]
Successfully stored pattern: pattern_66_20_67_20_68_20_69_20_4.00_
Total patterns processed: 1
Successfully stored: 1
Failed: 0
EOF

# Extract and normalize relevant lines from log file
grep -E "Added layer|Pattern input_layers size|Serialized input_layers JSON|Inserting polygon|Successfully stored pattern|Total patterns processed|Successfully stored|Failed" "${LOG_FILE}" | \
sed -E 's/Successfully stored pattern: pattern_66_20_67_20_68_20_69_20_4\.00_[0-9]+/Successfully stored pattern: pattern_66_20_67_20_68_20_69_20_4.00_/' > "${LOG_FILE}.filtered"

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

# Query database, excluding id from pattern_geometries
psql -d "${DB_NAME}" -c "SELECT id, pattern_hash, mask_layer_number, mask_layer_datatype, input_layers::text, layout_file_name FROM patterns;" -t -A > "${DB_OUTPUT_FILE}"
psql -d "${DB_NAME}" -c "SELECT pattern_id, layer_number, datatype, geometry_type, coordinates::text, area, perimeter FROM pattern_geometries ORDER BY layer_number, coordinates::text;" -t -A >> "${DB_OUTPUT_FILE}"

# Create expected database output
cat > "${EXPECTED_DB_FILE}" << 'EOF'
1|pattern_66_20_67_20_68_20_69_20_4.00_|66|20|[{"layer":67, "datatype":20}, {"layer":68, "datatype":20}, {"layer":69, "datatype":20}]|./test.gds
1|66|20|polygon|[[1, 1], [3, 1], [3, 3], [1, 3]]|4|8
1|67|20|polygon|[[1.20, 1.20], [1.80, 1.20], [1.80, 1.80], [1.20, 1.80]]|0.36|2.4
1|67|20|polygon|[[1.80, 1.80], [2.40, 1.80], [2.40, 2.40], [1.80, 2.40]]|0.36|2.4
1|68|20|polygon|[[1.20, 1.20], [1.60, 1.20], [1.40, 1.60]]|0.08|1.294427
1|68|20|polygon|[[1.28, 1.68], [1.68, 1.68], [1.48, 2.08]]|0.08|1.294427
1|68|20|polygon|[[1.64, 1.24], [2.04, 1.24], [1.84, 1.64]]|0.08|1.294427
1|69|20|polygon|[[1.40, 1.40], [1.80, 1.40], [1.80, 1.80], [1.40, 1.80]]|0.16|1.6
1|69|20|polygon|[[2, 1.40], [2.40, 1.40], [2.40, 1.80], [2, 1.80]]|0.16|1.6
EOF

# Filter out timestamp from pattern_hash and normalize JSON and coordinate formatting
sed -E 's/\|pattern_66_20_67_20_68_20_69_20_4\.00_[0-9]+/\|pattern_66_20_67_20_68_20_69_20_4.00_/; s/":\s*/":/g; s/\s*,"/,"/g; s/,\s+/, /g; s/\[\s*/[/g; s/\s*\]/]/g; s/([0-9])\.0+\b/\1/g; s/([0-9])\.0+\|/\1|/g; s/\|([0-9])\.0+\|/|\1|/g; s/":([0-9]+)\b/":\1/g' "${DB_OUTPUT_FILE}" > "${DB_OUTPUT_FILE}.filtered"

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
