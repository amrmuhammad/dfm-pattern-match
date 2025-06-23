#!/bin/bash

# Regression test for DFM Patterns Database Viewer Application

set -e # Exit on error

# Paths and variables
PROJECT_DIR="/home/amrmuhammad/dev/dfm_pattern_match4/dfm_viewer"
echo "${PROJECT_DIR}"
BUILD_DIR="${PROJECT_DIR}/build"
TEST_DIR="${PROJECT_DIR}/test/test_basic_1"
DB_NAME="test_db2"
LOG_FILE="${TEST_DIR}/test_output.log"
EXPECTED_LOG_FILE="${TEST_DIR}/expected_log.txt"
DB_OUTPUT_FILE="${TEST_DIR}/db_output.txt"
EXPECTED_DB_FILE="${TEST_DIR}/expected_db.txt"

export DFM_LOGGING=1
export DFM_LOG_LEVEL=DEBUG


# Run DFM Pattern Viewer
echo "Running DFMPatternViewer"
cd "${TEST_DIR}"
COMMAND="ddd ${BUILD_DIR}/DFMPatternViewer &"
echo "Executing: ${COMMAND}"
eval ${COMMAND} > "${LOG_FILE}" 2>&1 || {
    echo "Error: DFMPatternViewer failed with exit code $?"
    cat "${LOG_FILE}"
    exit 1
}


echo "Regression test passed successfully!"
exit 0
