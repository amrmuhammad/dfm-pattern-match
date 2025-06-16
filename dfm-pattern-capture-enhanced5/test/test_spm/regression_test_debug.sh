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

# Step 1: Clean up the database
echo "Cleaning up database: ${DB_NAME}"
psql -d "${DB_NAME}" -c "TRUNCATE TABLE pattern_geometries, patterns RESTART IDENTITY;" || {
    echo "Error: Failed to clean database"
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

# Step 3: Verify logs
echo "Verifying logs"
grep -E "Parsed input layer|Main: Input layers|Available layers|Loading layer|Added polygon|Bounding box|AND operation|Intersection valid|Successfully stored|Failed" "${LOG_FILE}" | \
sed -E 's/Successfully stored pattern: pattern_[0-9]+_[0-9]+\.[0-9]+_[0-9]+_/Successfully stored pattern: pattern_X_YYYY.YY_Z_/' > "${LOG_FILE}.filtered"

if [ -f "${EXPECTED_LOG_FILE}" ]; then
    if ! diff -u "${EXPECTED_LOG_FILE}" "${LOG_FILE}.filtered" > "${LOG_FILE}.diff"; then
        echo "Error: Log output does not match expected"
        cat "${LOG_FILE}.diff"
        exit 1
    else
        echo "Logs verified successfully"
    fi
else
    echo "Warning: No expected log file found at ${EXPECTED_LOG_FILE}. Creating it from current output."
    cp "${LOG_FILE}.filtered" "${EXPECTED_LOG_FILE}"
    echo "Please verify ${EXPECTED_LOG_FILE} manually."
fi

# Step 4: Check database entries
echo "Checking database entries"
psql -d "${DB_NAME}" -c "SELECT id, pattern_hash, mask_layer_number, input_layers::text, layout_file_name FROM patterns;" -t -A > "${DB_OUTPUT_FILE}"
psql -d "${DB_NAME}" -c "SELECT id, pattern_id, layer_number, geometry_type, coordinates::text, area, perimeter FROM pattern_geometries;" -t -A >> "${DB_OUTPUT_FILE}"

sed -E 's/\|pattern_[0-9]+_[0-9]+\.[0-9]+_[0-9]+_/\|pattern_X_YYYY.YY_Z_/; s/,\s+/,/g' "${DB_OUTPUT_FILE}" > "${DB_OUTPUT_FILE}.filtered"

if [ -f "${EXPECTED_DB_FILE}" ]; then
    if ! diff -u "${EXPECTED_DB_FILE}" "${DB_OUTPUT_FILE}.filtered" > "${DB_OUTPUT_FILE}.diff"; then
        echo "Error: Database entries do not match expected"
        cat "${DB_OUTPUT_FILE}.diff"
        exit 1
    else
        echo "Database entries verified successfully"
    fi
else
    echo "Warning: No expected database output file found at ${EXPECTED_DB_FILE}. Creating it from current output."
    cp "${DB_OUTPUT_FILE}.filtered" "${EXPECTED_DB_FILE}"
    echo "Please verify ${EXPECTED_DB_FILE} manually."
fi

# Clean up temporary files
rm -f "${LOG_FILE}" "${LOG_FILE}.filtered" "${LOG_FILE}.diff" "${DB_OUTPUT_FILE}" "${DB_OUTPUT_FILE}.filtered" "${DB_OUTPUT_FILE}.diff"

echo "Test completed successfully!"
exit 0
